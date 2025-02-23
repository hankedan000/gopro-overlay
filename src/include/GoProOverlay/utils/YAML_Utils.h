#pragma once

#include <opencv2/core/types.hpp>
#include <yaml-cpp/yaml.h>

#define YAML_TO_FIELD(yaml_node, field_name, field) field = yaml_node[field_name].as<decltype(field)>()
#define YAML_TO_FIELD_W_DEFAULT(yaml_node, field_name, field, DEFAULT_) field = yaml_node[field_name].as<decltype(field)>(DEFAULT_)

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

namespace YAML
{

	template<>
	struct convert<cv::Vec2d>
	{
		static Node
		encode(
			const cv::Vec2d& rhs)
		{
			Node node;
			node.push_back(rhs[0]);
			node.push_back(rhs[1]);

			return node;
		}

		static bool
		decode(
			const Node& node,
			cv::Vec2d& rhs)
		{
			if( ! node.IsSequence() || node.size() != 2) {
				return false;
			}

			rhs[0] = node[0].as<double>();
			rhs[1] = node[1].as<double>();

			return true;
		}
	};

	template<>
	struct convert<cv::Point>
	{
		static Node
		encode(
			const cv::Point& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);

			return node;
		}

		static bool
		decode(
			const Node& node,
			cv::Point& rhs)
		{
			if( ! node.IsSequence() || node.size() != 2) {
				return false;
			}

			YAML_TO_FIELD(node,0,rhs.x);
			YAML_TO_FIELD(node,1,rhs.y);

			return true;
		}
	};

	template<>
	struct convert<cv::Size>
	{
		static Node
		encode(
			const cv::Size& rhs)
		{
			Node node;
			node["width"] = rhs.width;
			node["height"] = rhs.height;

			return node;
		}

		static bool
		decode(
			const Node& node,
			cv::Size& rhs)
		{
			YAML_TO_FIELD(node,"width",rhs.width);
			YAML_TO_FIELD(node,"height",rhs.height);

			return true;
		}
	};

	template<>
	struct convert<cv::Scalar>
	{
		static Node
		encode(
			const cv::Scalar& rhs)
		{
			Node node;
			node.push_back(rhs[0]);
			node.push_back(rhs[1]);
			node.push_back(rhs[2]);
			node.push_back(rhs[3]);

			return node;
		}

		static bool
		decode(
			const Node& node,
			cv::Scalar& rhs)
		{
			if( ! node.IsSequence() || node.size() != 4) {
				return false;
			}

			rhs[0] = node[0].as<double>();
			rhs[1] = node[1].as<double>();
			rhs[2] = node[2].as<double>();
			rhs[3] = node[3].as<double>();

			return true;
		}
	};

}