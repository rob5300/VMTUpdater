#pragma once
#include <vector>
#include <string>
#include <map>
#include "../ext/json.hpp"
#include "OutputInfo.h"

class ImageSplitConfig
{
public:
	std::string imagesPath;
	std::map<std::string, std::vector<OutputInfo>> imageOutputs;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ImageSplitConfig, imagesPath, imageOutputs)