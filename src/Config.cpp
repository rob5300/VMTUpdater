#include "Config.h"
#include <fstream>
#include <ostream>
#include <map>

using json = nlohmann::json;
using namespace std;

#define NODE_MAPPINGS_FILENAME "nodemapping.json"
#define IMAGE_SPLIT_FILENAME "imageSplit.json"

Config LoadConfig(filesystem::path& workingPath, filesystem::path& configPath, bool& success)
{
	Config newConfig;
	if (filesystem::exists(configPath))
	{
		filesystem::path mappingsPath = configPath;
		mappingsPath /= NODE_MAPPINGS_FILENAME;

		if (filesystem::exists(mappingsPath))
		{
			ifstream f(mappingsPath);
			json data = json::parse(f);
			newConfig.nodeMappings = data.get<NodeMappingsConfig>();
			success = true;
		}
		else
		{
			//Create default
			newConfig.nodeMappings.shaderNodeName = "shader";
			newConfig.nodeMappings.shaderReplacementName = "Layer0";
			newConfig.nodeMappings.shaderMappings["VertexLitGeneric"] = "?";
			newConfig.nodeMappings.nodeMappings["$basetexture"] = "F_SPECULAR";
			newConfig.nodeMappings.nodesToRemove.insert("Proxies");
			
			json defaultNodeMappings = json(newConfig.nodeMappings);
			std::ofstream o(mappingsPath);
			o << std::setw(4) << defaultNodeMappings << std::endl;
			printf("Wrote default Node Mappings config to: %s\n", mappingsPath.string().c_str());
			success = false;
		}

		filesystem::path imageSplitPath = configPath;
		imageSplitPath /= IMAGE_SPLIT_FILENAME;

		if (filesystem::exists(imageSplitPath))
		{
			ifstream f(imageSplitPath);
			json data = json::parse(f);
			newConfig.imageSplit = data.get<ImageSplitConfig>();

			//Check if the images path exists
			filesystem::path imagesPath(newConfig.imageSplit.imagesPath);
			if (!imagesPath.is_absolute() || !filesystem::exists(imagesPath))
			{
				//Check if images path is relative to our working dir instead
				filesystem::path relativeImagesPath = workingPath;
				relativeImagesPath += imagesPath;
				relativeImagesPath = relativeImagesPath.lexically_normal();
				if (!filesystem::exists(relativeImagesPath))
				{
					printf("ERROR, Failed to verify the given images path '%s'", newConfig.imageSplit.imagesPath.c_str());
					success = false;
				}
				else
				{
					//Relative path was valid, write back updated path
					newConfig.imageSplit.imagesPath = relativeImagesPath.string();
				}
			}

			success = success && true;
		}
		else
		{
			//Create default
			OutputInfo specularInfo;
			specularInfo.inputChannel = A;
			specularInfo.outputChannels = 3;
			specularInfo.outputImageName = "specular.png";
			specularInfo.requiredNode = "$phong";
			specularInfo.requiredNodeValue = "1";

			std::vector<OutputInfo> outputList;
			outputList.push_back(specularInfo);
			newConfig.imageSplit.imageOutputs["$basetexture"] = outputList;
			json defaultImageSplits = json(newConfig.imageSplit);
			std::ofstream o(imageSplitPath);
			o << std::setw(4) << defaultImageSplits << std::endl;
			printf("Wrote default ImageSplit config to: %s\n", imageSplitPath.string().c_str());
			success = false;
		}
	}
	else
	{
		printf("Input folder not found, cannot load config files");
		success = false;
	}

	return newConfig;
}

std::vector<OutputInfo>* Config::GetImageOutputInfos(string& nodeName)
{
	if (this->imageSplit.imageOutputs.contains(nodeName))
	{
		return &this->imageSplit.imageOutputs[nodeName];
	}
	return nullptr;
}

std::string Config::GetNodeMapping(string& nodeName)
{
	if (nodeMappings.nodeMappings.contains(nodeName))
	{
		return nodeMappings.nodeMappings.at(nodeName);
	}
}

std::string Config::GetShaderMapping(string& nodeName)
{
	if (nodeMappings.shaderMappings.contains(nodeName))
	{
		return nodeMappings.shaderMappings.at(nodeName);
	}
}

bool Config::ShouldNodeBeRemoved(string& nodeName)
{
	return nodeMappings.nodesToRemove.contains(nodeName);
}