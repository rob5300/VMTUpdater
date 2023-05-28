#include "VMTNodeUpdater.h"
#include <map>
#include <string>
#include "VTFLib.h"
#include <ImageSplitter.h>

using namespace VTFLib::Nodes;
using namespace VTFLib;
using namespace std;

Config* config;

inline filesystem::path FixVMTPath(const char* nodeImageName)
{
	filesystem::path imagePath(config->imageSplit.imagesPath);
	imagePath /= nodeImageName;
	if (!imagePath.has_extension())
	{
		imagePath += ".vtf";
	}

	if (!filesystem::exists(imagePath))
	{
		//Try again but just use images path + filename
		filesystem::path nodeNameAsPath(nodeImageName);
		imagePath = filesystem::path(config->imageSplit.imagesPath);
		imagePath /= nodeNameAsPath.filename();
		if (!imagePath.has_extension())
		{
			imagePath += ".vtf";
		}
	}

	return imagePath;
}

void UpdateNodeNames(CVMTFile& vmtFile);

void UpdateVMTNodes(filesystem::path path, Config& _config)
{
	config = &_config;
	CVMTFile vmtFile;
	auto pathstr = path.string();
	printf("- Load %s\n", pathstr.c_str());
	vmtFile.Load(pathstr.c_str());

	//Remove proxy data
	auto rootNode = vmtFile.GetRoot();
	auto proxyNode = rootNode->GetNode("Proxies");
	if (proxyNode != nullptr)
	{
		rootNode->RemoveNode(proxyNode);
	}

	UpdateNodeNames(vmtFile);

	//construct new path and save
	auto newPath = path.parent_path();
	newPath /= "updated";
	newPath /= path.stem();
	newPath += config->nodeMappings.extension;
	auto newPathstr = newPath.string();
	bool saved = vmtFile.Save(newPathstr.c_str());
	if (saved)
	{
		printf("- Saved to %s\n", newPathstr.c_str());
	}
	else
	{
		printf("- FAILED to save to %s", newPathstr.c_str());
	}
}

void UpdateNodeNames(CVMTFile& vmtFile)
{
	auto root = vmtFile.GetRoot();
	//Copy old name
	string oldShader = string(root->GetName());
	root->SetName("Layer0");

	string newShaderName = config->GetShaderMapping(oldShader);
	if (newShaderName.length() != 0)
	{
		root->AddStringNode("shader", newShaderName.c_str());
		printf("- Update shader from %s -> %s\n", oldShader.c_str(), newShaderName.c_str());
	}

	vector<pair<string, string>> nodesToModify;

	int32_t nodeCount = root->GetNodeCount();
	for (int i = 0; i < nodeCount; i++)
	{
		auto node = root->GetNode(i);
		string nodeName = string(node->GetName());
		CVMTStringNode* stringNode = static_cast<CVMTStringNode*>(node);
		auto imageSplitInfos = config->GetImageOutputInfos(nodeName);

		if (imageSplitInfos != nullptr && imageSplitInfos->size() != 0)
		{
			//Filter OutputInfos before running image splitter
			for (int32_t i = imageSplitInfos->size() - 1; i >= 0; i--)
			{
				OutputInfo* outputInfo = &imageSplitInfos->at(i);
				if (!outputInfo->requiredNode.empty())
				{
					//Verify that a node exists and its value matches the required val
					auto requiredNode = root->GetNode(outputInfo->requiredNode.c_str());
					if (requiredNode != nullptr)
					{
						auto requiredNodeCStr = static_cast<CVMTStringNode*>(requiredNode)->GetValue();
						//Does the node value match the configured value?
						//If not, erase this info
						if (strcmp(requiredNodeCStr, outputInfo->requiredNodeValue.c_str()) != 0)
						{
							imageSplitInfos->erase(imageSplitInfos->begin() + i);
						}
					}
				}
			}

			if (imageSplitInfos->size() != 0)
			{
				//Make and execute image splitter
				printf("- Will run splitter for %i valid matches\n", (int)imageSplitInfos->size());
				filesystem::path imagePath = FixVMTPath(stringNode->GetValue());
				ImageSplitter splitter(imageSplitInfos, imagePath.string(), 4);
				bool success = splitter.Split(nodesToModify);
				printf("- Split success? %s\n", success ? "yes" : "no");
			}
		}

		string newNodeName = config->GetNodeMapping(nodeName);
		if (!newNodeName.empty())
		{
			//Update node name
			stringNode->SetName(newNodeName.c_str());
			printf("- Update %s name to -> %s\n", nodeName.c_str(), newNodeName.c_str());
		}

		if (config->ShouldNodeBeRemoved(nodeName))
		{
			root->RemoveNode(node);
			printf("- Removed node %s\n", nodeName.c_str());
		}
	}

	//Add new nodes
	printf("- %i nodes to change from image splitting:\n", nodesToModify.size());
	for (const auto& newNodePair : nodesToModify)
	{
		auto existingNode = root->GetNode(newNodePair.first.c_str());
		if (existingNode != nullptr)
		{
			static_cast<CVMTStringNode*>(existingNode)->SetValue(newNodePair.second.c_str());
			printf("- Update existing node '%s' -> %s\n", newNodePair.first.c_str(), newNodePair.second.c_str());
		}
		else
		{
			root->AddStringNode(newNodePair.first.c_str(), newNodePair.second.c_str());
			printf("- Insert new node '%s' with value %s\n", newNodePair.first.c_str(), newNodePair.second.c_str());
		}
	}
}