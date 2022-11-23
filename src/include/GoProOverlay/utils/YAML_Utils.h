#pragma once

#include <yaml-cpp/yaml.h>

#define YAML_TO_FIELD(yaml_node, field_name, field) field = yaml_node[field_name].as<decltype(field)>()

namespace utils
{
	template <class T>
	void
	yamlToVector(
		const YAML::Node &list_node,
		std::vector<T> &out_vector)
	{
		out_vector.resize(list_node.size());
		for (unsigned int i=0; i<list_node.size(); i++)
		{
			out_vector[i] = list_node[i].as<T>();
		}
	}

	template <class T>
	void
	vectorToYaml(
		const std::vector<T> &vector,
		YAML::Node node)
	{
		for (auto item : vector)
		{
			node.push_back(item);
		}
	}
}