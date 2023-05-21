#pragma once
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

	//Input channel
	Channel inputChannel;

	int32_t outputChannels;

	//Output image name + extension
	std::string outputImageName;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(OutputInfo, inputChannel, outputChannels, outputImageName, requiredNode, requiredNodeValue)