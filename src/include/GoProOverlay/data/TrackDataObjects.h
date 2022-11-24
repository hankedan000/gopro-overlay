#pragma once

#include <opencv2/opencv.hpp>
#include <tuple>
#include <vector>
#include <yaml-cpp/yaml.h>

#include "GoProOverlay/utils/YAML_Utils.h"
#include "TelemetrySource.h"

namespace gpo
{

	class DetectionGate
	{
	public:
		DetectionGate();

		DetectionGate(
			cv::Vec2d a,
			cv::Vec2d b);
		
		bool
		detect(
			cv::Vec2d c1,
			cv::Vec2d c2);

		const cv::Vec2d &
		a() const;

		const cv::Vec2d &
		b() const;

		cv::Vec2d &
		a();

		cv::Vec2d &
		b();

	private:
		cv::Vec2d a_;
		cv::Vec2d b_;
	};

	struct Sector
	{
		std::string name;
		DetectionGate entry;
		DetectionGate exit;
	};

	// conversion factor from decimal degrees to meter (on earth)
	const double DECDEG_PER_METER = 1.0 / 111000.0;

	inline
	double
	dd2m(
		double dd)
	{
		return dd / DECDEG_PER_METER;
	}

	inline
	double
	m2dd(
		double m)
	{
		return m * DECDEG_PER_METER;
	}

	class Track
	{
	public:
		Track();

		Track(
			const std::vector<cv::Vec2d> &path);

		void
		setStart(
			const DetectionGate &start);

		const DetectionGate &
		getStart() const;

		void
		setFinish(
			const DetectionGate &finish);

		const DetectionGate &
		getFinish() const;

		void
		addSector(
			const Sector &s);

		bool
		removeSector(
			size_t idx);

		Sector &
		getSector(
			size_t idx);

		const Sector &
		getSector(
			size_t idx) const;

		size_t
		sectorCount() const;

		size_t
		pathCount() const;

		cv::Vec2d
		getPathPoint(
			size_t idx) const;

		DetectionGate
		getDetectionGate(
			size_t pathIdx,
			double width_meters) const;

		DetectionGate
		getNearestDetectionGate(
			cv::Vec2d p,
			double width_meters) const;

		std::pair<bool,cv::Vec2d>
		findClosestPoint(
			cv::Vec2d p) const;

		std::tuple<bool,cv::Vec2d, size_t>
		findClosestPointWithIdx(
			cv::Vec2d p) const;

	private:
		DetectionGate start_;
		DetectionGate finish_;
		std::vector<Sector> sectors_;
		// list of lat/lon points making up the track's path
		std::vector<cv::Vec2d> path_;

	};

	Track
	makeTrackFromTelemetry(
		TelemetrySourcePtr tSrc);

}

// overrides for yaml config encode/decode
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

		node[0].as<double>(rhs[0]);
		node[1].as<double>(rhs[1]);

		return true;
	}
};

template<>
struct convert<gpo::DetectionGate>
{
	static Node
	encode(
		const gpo::DetectionGate& rhs)
	{
		Node node;
		node["a"] = rhs.a();
		node["b"] = rhs.b();

		return node;
	}

	static bool
	decode(
		const Node& node,
		gpo::DetectionGate& rhs)
	{
		node["a"].as<cv::Vec2d>(rhs.a());
		node["b"].as<cv::Vec2d>(rhs.b());

		return true;
	}
};

template<>
struct convert<gpo::Sector>
{
	static Node
	encode(
		const gpo::Sector& rhs)
	{
		Node node;
		node["name"] = rhs.name;
		node["entry"] = rhs.entry;
		node["exit"] = rhs.exit;

		return node;
	}

	static bool
	decode(
		const Node& node,
		gpo::Sector& rhs)
	{
		node["name"].as<std::string>(rhs.name);
		node["entry"].as<gpo::DetectionGate>(rhs.entry);
		node["exit"].as<gpo::DetectionGate>(rhs.exit);

		return true;
	}
};

template<>
struct convert<gpo::Track>
{
	static Node
	encode(
		const gpo::Track& rhs)
	{
		Node node;
		node["start"] = rhs.getStart();
		node["finish"] = rhs.getFinish();
		for (size_t ss=0; ss<rhs.sectorCount(); ss++)
		{
			node["sectors"].push_back(rhs.getSector(ss));
		}

		return node;
	}

	static bool
	decode(
		const Node& node,
		gpo::Track& rhs)
	{
		rhs.setStart(node["start"].as<gpo::DetectionGate>());
		rhs.setFinish(node["finish"].as<gpo::DetectionGate>());
		YAML::Node sectors = node["sectors"];
		if(sectors && sectors.IsSequence())
		{
			for (size_t ss=0; ss<sectors.size(); ss++)
			{
				gpo::Sector sector = sectors[ss].as<gpo::Sector>();
				rhs.addSector(sector);
			}
			return false;
		}

		return true;
	}
};

}// namespace YAML