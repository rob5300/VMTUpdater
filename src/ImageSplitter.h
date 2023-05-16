#pragma once
#include <vector>
#include <string>
#include "IL\il.h"
#include "../ext/VTFLib.h"

enum Channel {R = 0, G = 1, B = 2, A = 3};

class OutputInfo
{
public:
	//Input channel
	Channel inputChannel;

	ILuint outputChannels;

	//Output image name + extension
	std::wstring outputImageName;

	//Output image format as DevIL constant
	ILenum outputFormat;
};

class ImageSplitter
{
public:
	ImageSplitter(OutputInfo* outputInfos, size_t outputInfosCount, const char* inputImagePath, char inputImgChannels);
	//Split image based on output infos given
	bool Split();

private:
	//Get data (convert to rgba if needed)
	vlByte* ResolveFormat(VTFLib::CVTFFile& vtfFile);

	//Input image path
	std::string inputImagePath;

	//Channels of input image (e.g. 3)
	ILuint inputImgChannels;

	std::vector<OutputInfo> outputInfos;

	vlByte* convertedVTF;
};