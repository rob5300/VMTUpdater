#pragma once
#include <string>
#include <map>
#include <vector>
#include <unordered_set>
#include "../ext/json.hpp"

class NodeMappingsConfig
{
public:
	std::string shaderNodeName;
	std::string shaderReplacementName;
	std::unordered_set<std::string> nodesToRemove;
	nlohmann::json nodeMappings;
	nlohmann::json shaderMappings;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(NodeMappingsConfig, shaderNodeName, shaderReplacementName, nodesToRemove, nodeMappings, shaderMappings)
};