#pragma once
#include <vector>
#include <string>
#include <utility>
#include "IL\il.h"
#include "OutputInfo.h"
#include "../ext/json.hpp"
#include "../ext/VTFLib.h"

class ImageSplitter
{
public:
	ImageSplitter(std::vector<OutputInfo>* outputInfos, const std::string& inputImagePath, char inputImgChannels);
	//Split image based on output infos given
	bool Split(std::vector<std::pair<std::string, std::string>> &nodesToModify);

private:
	//Get data (convert to rgba if needed)
	vlByte* ResolveFormat(VTFLib::CVTFFile& vtfFile);

	//Input image path
	std::string inputImagePath;

	//Channels of input image (e.g. 3)
	ILuint inputImgChannels;

	std::vector<OutputInfo>* outputInfos;

	vlByte* convertedVTF;
};