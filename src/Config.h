#pragma once
#include <filesystem>
#include <vector>
#include <string>
#include <map>
#include <set>
#include "../ext/json.hpp"
#include "ImageSplitConfig.h"
#include "NodeMappingsConfig.h"

struct Config
{
public:
	NodeMappingsConfig nodeMappings;
	ImageSplitConfig imageSplit;

	std::vector<OutputInfo>* GetImageOutputInfos(std::string& nodeName);
	std::string GetNodeMapping(std::string& nodeName);
	std::string GetShaderMapping(std::string& nodeName);
	bool ShouldNodeBeRemoved(std::string& nodeName);
};

Config LoadConfig(std::filesystem::path& workingPath, std::filesystem::path& configPath, bool& success);