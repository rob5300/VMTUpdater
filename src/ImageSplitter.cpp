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

inline void CopyPixel(int x, int y, unsigned char* input, ILubyte* buffer, OutputInfo& outputInfo, ILenum format, ILuint channels)
{
	auto inputColor = input + outputInfo.inputChannel;
	//Set colours of buffer
	for (size_t i = 0; i < channels; i++)
	{
		buffer[i] = *inputColor;
	}

	ilSetPixels(x, y, 0, 1, 1, 1, format, IL_UNSIGNED_BYTE, buffer);
}

ImageSplitter::ImageSplitter(OutputInfo* outputInfos, size_t outputInfosCount, const char* inputImagePath, char inputImgChannels)
{
	this->outputInfos.assign(outputInfos, outputInfos + outputInfosCount);
	this->inputImgChannels = inputImgChannels;

	this->inputImagePath = std::string(inputImagePath);
}

bool ImageSplitter::Split()
{
	if (outputInfos.size() == 0)
	{
		printf("No output infos given");
		return false;
	}

	CVTFFile vtfFile;
	bool success = vtfFile.Load(inputImagePath.c_str(), false);
	if (!success)
	{
		printf("Failed to load VTF at '%s'", inputImagePath.c_str());
		return false;
	}

	auto width = vtfFile.GetWidth();
	auto height = vtfFile.GetHeight();

	auto inputData = ResolveFormat(vtfFile);

	if (inputData == nullptr)
	{
		return false;
	}

	ILubyte* copyBuffer = new ILubyte[4];

	//Prepare to create new images
	size_t outputInfosCount = outputInfos.size();
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
		OutputInfo* info = &outputInfos[i];
		ilBindImage(imageIds[i]);
		ilActiveImage(imageIds[i]);
		ilActiveLayer(0);
		ILenum format = info->outputChannels == 4 ? IL_RGBA : IL_RGB;
		ilTexImage(width, height, 1, info->outputChannels, format, IL_UNSIGNED_BYTE, NULL);

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				int index = GetIndexForPosition(x, y, width, height, inputImgChannels);
				auto pixel = &inputData[index];
				CopyPixel(x, y, pixel, copyBuffer, *info, format, info->outputChannels);
			}
		}
		
		//Save new image
		path newPath = outputPath;
		newPath /= info->outputImageName;
		auto newPathAsString = newPath.string();
		if (exists(newPath))
		{
			remove(newPath);
		}

		bool saveSuccess = ilSave(info->outputFormat, newPathAsString.c_str());
		if (saveSuccess)
		{
			printf("Saving new image: %s", newPathAsString.c_str());
		}
		else
		{
			printf("FAILED to save new image to %s", newPathAsString.c_str());
			return false;
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
			printf("VTF type missmatch (%s). Expected %i channels", rgb8 ? "rgb8" : "rgba8", inputImgChannels);
			return nullptr;
		}
		else
		{
			//VTF already usable
			return vtfFile.GetData(0, 0, 0, 0);
		}
	}

	convertedVTF = new vlByte[w * h * 4];
	inputImgChannels = 4;
	vtfFile.ConvertToRGBA8888(vtfFile.GetData(0,0,0,0), convertedVTF, w, h, format);
	return convertedVTF;
}
