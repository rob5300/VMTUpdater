#pragma once
#include <vector>
#include "../ext/json.hpp"

enum Channel {R = 0, G = 1, B = 2, A = 3};

NLOHMANN_JSON_SERIALIZE_ENUM(Channel, {
    {R, "R"},
    {G, "G"},
    {B, "B"},
    {A, "A"},
})

struct OutputInfo
{
public:
	//Check if a node exists
	std::string requiredNode;
	//Check if an existing node has a value
	std::string requiredNodeValue;

	//Channel Mapping
	std::vector<Channel> channelMapping;

	//Output image name + extension
	std::string outputImageName;

	//Node to insert/modify and add new file name to
	std::string nodeToCreate;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(OutputInfo, channelMapping, outputImageName, requiredNode, requiredNodeValue, nodeToCreate)