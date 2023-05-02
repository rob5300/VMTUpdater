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
	vmtFile.Load(path.string().c_str());

	UpdateNodeNames(vmtFile);


	//vmtFile.Save()
}

void UpdateNodeNames(CVMTFile& vmtFile)
{
	auto root = vmtFile.GetRoot();
	//Copy old name
	string oldShader = string(root->GetName());
	root->SetName("Layer0");

	if (shaderMappings.contains(oldShader))
	{
		root->AddStringNode("shader", shaderMappings[oldShader].c_str());
	}

	int32_t nodeCount = root->GetNodeCount();
	for (int i = 0; i < nodeCount; i++)
	{
		auto node = root->GetNode(i);

		auto nodeName = node->GetName();
		if (nodeMappings.contains(nodeName))
		{
			static_cast<CVMTStringNode*>(node)->SetValue(nodeMappings[nodeName].c_str());
		}
	}
}