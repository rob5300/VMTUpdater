#include "ImageSplitter.h"
#include <algorithm>
#include <filesystem>
#include <VTFLib.h>

using namespace std::filesystem;
using namespace VTFLib;
using namespace std;

inline int GetIndexForPosition(int x, int y, ILint width, ILint height, int channels)
{
	return (x + y * width) * channels;
}

inline void CopyPixel(int x, int y, unsigned char* input, ILubyte* buffer, OutputInfo& outputInfo, ILenum format)
{
	//Set colours of buffer based on info mapping
	for (size_t i = 0; i < outputInfo.channelMapping.size(); i++)
	{
		buffer[i] = *(input + outputInfo.channelMapping[i]);
	}

	ilSetPixels(x, y, 0, 1, 1, 1, format, IL_UNSIGNED_BYTE, buffer);
}

ImageSplitter::ImageSplitter(std::vector<OutputInfo>* outputInfos, const std::string& inputImagePath, char inputImgChannels)
{
	this->outputInfos = outputInfos;
	this->inputImgChannels = inputImgChannels;

	this->inputImagePath = std::string(inputImagePath);
}

bool ImageSplitter::Split(std::vector<std::pair<std::string, std::string>> &nodesToModify)
{
	if (outputInfos->size() == 0)
	{
		printf("ERROR, No output infos given");
		return false;
	}

	CVTFFile vtfFile;
	bool success = vtfFile.Load(inputImagePath.c_str(), false);
	if (!success)
	{
		printf("ERROR, Failed to load VTF at '%s'", inputImagePath.c_str());
		return false;
	}

	auto width = vtfFile.GetWidth();
	auto height = vtfFile.GetHeight();

	bool hasImage = vtfFile.GetHasImage();
	if (!hasImage) return false;

	bool alphaflag = vtfFile.GetFlag(TEXTUREFLAGS_EIGHTBITALPHA);
	if (!alphaflag) return false;

	auto inputData = ResolveFormat(vtfFile);

	if (inputData == nullptr)
	{
		return false;
	}

	ILubyte* copyBuffer = new ILubyte[4];

	//Prepare to create new images
	size_t outputInfosCount = outputInfos->size();
	ILuint* imageIds = new ILuint[outputInfosCount];
	ilGenImages(outputInfosCount, imageIds);

	path outputPath(inputImagePath);
	outputPath = outputPath.parent_path();
	outputPath /= "split_images";
	if (!is_directory(outputPath))
	{
		create_directory(outputPath);
	}
	for (size_t i = 0; i < outputInfosCount; i++)
	{
		OutputInfo* info = &this->outputInfos->at(i);
		ILuint channelCount = info->channelMapping.size();

		//If no mapping/ invalid mapping size, default to RGB
		if (channelCount <= 0 || channelCount > 4)
		{
			info->channelMapping = {R, G, B};
		}

		//Make filename original name if empty
		if (info->outputImageName.empty())
		{
			info->outputImageName = path(inputImagePath).stem().string();
		}

		ilBindImage(imageIds[i]);
		ilActiveImage(imageIds[i]);
		ilActiveLayer(0);
		
		ILenum format = channelCount == 4 ? IL_RGBA : IL_RGB;
		ilTexImage(width, height, 1, channelCount, format, IL_UNSIGNED_BYTE, NULL);

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				int index = GetIndexForPosition(x, y, width, height, inputImgChannels);
				auto pixel = &inputData[index];
				CopyPixel(x, y, pixel, copyBuffer, *info, format);
			}
		}
		
		//Save new image
		path newPath = outputPath;
		string oldVtfName = path(inputImagePath).stem().string();
		string newFileName = (oldVtfName + "_" + info->outputImageName);
		newPath /= newFileName;
		auto newPathAsString = newPath.string();
		if (exists(newPath))
		{
			remove(newPath);
		}

		bool saveSuccess = ilSave(IL_PNG, newPathAsString.c_str());
		if (saveSuccess)
		{
			printf("Saving new image: %s\n", newPathAsString.c_str());
		}
		else
		{
			printf("FAILED to save new image to %s\n", newPathAsString.c_str());
			return false;
		}

		if (!info->nodeToCreate.empty())
		{
			nodesToModify.push_back({info->nodeToCreate, newFileName});
		}
	}

	//Delete
	ilDeleteImages(outputInfosCount, imageIds);
	delete[] copyBuffer;
	delete[] imageIds;

	//Delete converted pixels if used
	if (convertedVTF != nullptr)
	{
		delete[] convertedVTF;
	}
}

vlByte* ImageSplitter::ResolveFormat(CVTFFile& vtfFile)
{
	auto format = vtfFile.GetFormat();
	auto w = vtfFile.GetWidth();
	auto h = vtfFile.GetHeight();

	bool rgb8 = format == IMAGE_FORMAT_RGB888;
	bool rgba8 = format == IMAGE_FORMAT_RGBA8888;

	if (rgb8 || rgba8)
	{
		if (rgb8 && inputImgChannels != 3 || rgba8 && inputImgChannels != 4)
		{
			printf("VTF type missmatch (%s). Expected %i channels\n", rgb8 ? "rgb8" : "rgba8", inputImgChannels);
			return nullptr;
		}
		else
		{
			//VTF already usable
			return vtfFile.GetData(0, 0, 0, 0);
		}
	}

	int32_t totalSize = w * h * 4;
	convertedVTF = new vlByte[totalSize];
	inputImgChannels = 4;
	auto vtfData = vtfFile.GetData(0, 0, 0, 0);
	if (vtfData != nullptr)
	{
		bool success = vtfFile.ConvertToRGBA8888(vtfData, convertedVTF, w, h, format);
		return convertedVTF;
	}
	return nullptr;
}
