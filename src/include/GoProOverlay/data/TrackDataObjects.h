#pragma once

#include <opencv2/opencv.hpp>
#include <tuple>
#include <vector>
#include <yaml-cpp/yaml.h>

#include "GoProOverlay/utils/YAML_Utils.h"
#include "TelemetrySource.h"

namespace gpo
{

	const double DEFAULT_GATE_WIDTH = 10.0;// 10m

	// forward declarations
	class Track;

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

	class TrackPathObject
	{
	public:
		TrackPathObject(
			Track *track,
			std::string name);

		Track*
		getTrack();

		virtual
		bool
		isGate() const;

		virtual
		bool
		isSector() const;

		virtual
		size_t
		getEntryIdx() const = 0;

		virtual
		size_t
		getExitIdx() const = 0;

		virtual
		DetectionGate
		getEntryGate() const = 0;

		virtual
		DetectionGate
		getExitGate() const = 0;

		const std::string &
		getName() const;

		void
		setName(
			std::string name);

		// YAML encode/decode
		virtual
		YAML::Node
		encode() const = 0;

		virtual
		bool
		decode(
			const YAML::Node& node) = 0;

	protected:
		Track* track_;
		std::string name_;

	};


	class TrackSector : public TrackPathObject
	{
	public:
		TrackSector(
			Track *track,
			std::string name,
			size_t entryIdx,
			size_t exitIdx);

		TrackSector(
			Track *track,
			std::string name,
			size_t entryIdx,
			size_t exitIdx,
			double gateWidth_meters);

		void
		setWidth(
			double width_meters);

		double
		getWidth() const;

		virtual
		bool
		isSector() const override;

		void
		setEntryIdx(
			size_t pathIdx);

		virtual
		size_t
		getEntryIdx() const override;

		void
		setExitIdx(
			size_t pathIdx);

		virtual
		size_t
		getExitIdx() const override;

		virtual
		DetectionGate
		getEntryGate() const override;

		virtual
		DetectionGate
		getExitGate() const override;

		// YAML encode/decode
		virtual
		YAML::Node
		encode() const override;

		virtual
		bool
		decode(
			const YAML::Node& node) override;

	private:
		size_t entryIdx_;
		size_t exitIdx_;
		double gateWidth_meters_;

	};

	class TrackGate : public TrackPathObject
	{
	public:
		TrackGate(
			Track *track,
			std::string name,
			size_t pathIdx);

		TrackGate(
			Track *track,
			std::string name,
			size_t pathIdx,
			double gateWidth_meters);

		void
		setWidth(
			double width_meters);

		double
		getWidth() const;

		virtual
		bool
		isGate() const override;

		void
		setPathIdx(
			size_t pathIdx);

		virtual
		size_t
		getEntryIdx() const override;

		virtual
		size_t
		getExitIdx() const override;

		virtual
		DetectionGate
		getEntryGate() const override;

		virtual
		DetectionGate
		getExitGate() const override;

		// YAML encode/decode
		virtual
		YAML::Node
		encode() const override;

		virtual
		bool
		decode(
			const YAML::Node& node) override;

	private:
		size_t pathIdx_;
		double gateWidth_meters_;

	};

	class Track
	{
	public:
		Track();

		Track(
			const std::vector<cv::Vec2d> &path);

		~Track();

		// start/finish related methods
		void
		setStart(
			size_t pathIdx);

		const TrackGate *
		getStart();

		void
		setFinish(
			size_t pathIdx);

		const TrackGate *
		getFinish();

		// sector related methods
		void
		addSector(
			std::string name,
			size_t entryIdx,
			size_t exitIdx);

		void
		removeSector(
			size_t idx);

		void
		setSectorName(
			size_t idx,
			std::string name);

		void
		setSectorEntry(
			size_t idx,
			size_t entryIdx);

		void
		setSectorExit(
			size_t idx,
			size_t exitIdx);

		const
		TrackSector *
		getSector(
			size_t idx);

		size_t
		sectorCount() const;

		// path related methods
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

		// YAML encode/decode
		YAML::Node
		encode() const;

		bool
		decode(
			const YAML::Node& node);

	private:
		TrackGate *start_;
		TrackGate *finish_;
		std::vector<TrackSector *> sectors_;
		// list of lat/lon points making up the track's path
		std::vector<cv::Vec2d> path_;

	};

	Track *
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

		rhs[0] = node[0].as<double>();
		rhs[1] = node[1].as<double>();

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
		rhs.a() = node["a"].as<cv::Vec2d>();
		rhs.b() = node["b"].as<cv::Vec2d>();

		return true;
	}
};

}// namespace YAML