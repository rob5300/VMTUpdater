#include "VMTNodeUpdater.h"
#include <map>
#include <string>
#include "VTFLib.h"

using namespace VTFLib::Nodes;
using namespace VTFLib;
using namespace std;

map<string, string> nodeMappings
{
	{"$basetexture", "TextureColor"},
	{"$phong", "F_SPECULAR"},
};

map<string, string> shaderMappings
{
	{"VertexLitGeneric", "shaders/vertexlit.shader"}
};

void UpdateNodeNames(CVMTFile& vmtFile);

void UpdateVMTNodes(filesystem::path path)
{
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
	newPath /= path.filename();
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

	if (shaderMappings.find(oldShader) != shaderMappings.end())
	{
		auto newShader = shaderMappings[oldShader].c_str();
		root->AddStringNode("shader", newShader);
		printf("- Update shader from %s -> %s\n", oldShader.c_str(), newShader);
	}

	int32_t nodeCount = root->GetNodeCount();
	for (int i = 0; i < nodeCount; i++)
	{
		auto node = root->GetNode(i);
		string nodeName = string(node->GetName());
		if (nodeMappings.find(nodeName) != nodeMappings.end())
		{
			auto newVal = nodeMappings[nodeName].c_str();
			static_cast<CVMTStringNode*>(node)->SetName(newVal);
			printf("- Update %s name to -> %s\n", nodeName.c_str(), newVal);
		}
	}
}