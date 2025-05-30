#pragma once

#include <memory>
#include <opencv2/core/matx.hpp> // for cv::Vec2d
#include <tuple>
#include <vector>
#include <yaml-cpp/yaml.h>

#include "GoProOverlay/data/ModifiableObject.h"
#include "GoProOverlay/utils/YAML_Utils.h"// for YAML::convert<cv::Vec2d>
#include "TelemetrySource.h"

namespace gpo
{

	const double DEFAULT_GATE_WIDTH = 10.0;// 10m

	// forward declarations
	class Track;
	using TrackPtr = std::shared_ptr<Track>;

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
			cv::Vec2d c2) const;

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

	enum GateType_E
	{
		eGT_Start,
		eGT_Finish,
		eGT_Other,
		eGT_NOT_A_GATE
	};

	class TrackPathObject
	{
	public:
		TrackPathObject(
			const Track *track,
			const std::string &name);
		
		virtual
		~TrackPathObject() = default;

		const Track *
		getTrack() const;

		virtual
		bool
		isGate() const;

		virtual
		bool
		isSector() const;

		virtual
		GateType_E
		getGateType() const;

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
			const std::string &name);

		// YAML encode/decode
		virtual
		YAML::Node
		encode() const = 0;

		virtual
		bool
		decode(
			const YAML::Node& node) = 0;

	private:
		const Track *track_;
		std::string name_;

	};


	class TrackSector : public TrackPathObject
	{
	public:
		TrackSector(
			const Track *track,
			const std::string &name,
			size_t entryIdx,
			size_t exitIdx);

		TrackSector(
			const Track *track,
			const std::string &name,
			size_t entryIdx,
			size_t exitIdx,
			double gateWidth_meters);

		void
		setWidth(
			double width_meters);

		double
		getWidth() const;

		bool
		isSector() const final;

		void
		setEntryIdx(
			size_t pathIdx);

		size_t
		getEntryIdx() const final;

		void
		setExitIdx(
			size_t pathIdx);

		size_t
		getExitIdx() const final;

		DetectionGate
		getEntryGate() const final;

		DetectionGate
		getExitGate() const final;

		// YAML encode/decode
		YAML::Node
		encode() const final;

		bool
		decode(
			const YAML::Node& node) final;

	private:
		size_t entryIdx_;
		size_t exitIdx_;
		double gateWidth_meters_;

	};

	class TrackGate : public TrackPathObject
	{
	public:
		TrackGate(
			const Track *track,
			const std::string &name,
			size_t pathIdx,
			GateType_E type);

		TrackGate(
			const Track *track,
			const std::string &name,
			size_t pathIdx,
			GateType_E type,
			double gateWidth_meters);

		void
		setWidth(
			double width_meters);

		double
		getWidth() const;

		bool
		isGate() const final;

		GateType_E
		getGateType() const final;

		void
		setPathIdx(
			size_t pathIdx);

		size_t
		getEntryIdx() const final;

		size_t
		getExitIdx() const final;

		DetectionGate
		getEntryGate() const final;

		DetectionGate
		getExitGate() const final;

		// YAML encode/decode
		YAML::Node
		encode() const final;

		bool
		decode(
			const YAML::Node &node) final;

	private:
		size_t pathIdx_;
		GateType_E type_;
		double gateWidth_meters_;

	};

	class Track : public ModifiableObject
	{
	public:
		enum RetCode
		{
			SUCCESS = 0,
			E_SECTOR_NO_WIDTH = -10,
			E_EXIT_BEFORE_ENTRY = -11,
			E_OVERLAP = -12
		};

	public:
		Track();

		explicit
		Track(
			const std::vector<cv::Vec2d> &path);

		// start/finish related methods
		void
		setStart(
			size_t pathIdx);

		const TrackGate &
		getStart() const;

		void
		setFinish(
			size_t pathIdx);

		const TrackGate &
		getFinish() const;

		// sector related methods
		/**
		 * Finds the location of where a sector should be inserted.
		 * This method can also be used to test if a sector is valid.
		 * 
		 * @param[in] entryIdx
		 * the sector's entry point on the path
		 * 
		 * @param[in] exitIdx
		 * the sector's exit point on the path
		 * 
		 * @return
		 * 'first' is the return code
		 *   SUCCESS is sector can be inserted at the location
		 *   E_SECTOR_NO_WIDTH if entry and exit are equal
		 *   E_EXIT_BEFORE_ENTRY if exitIdx falls before entryIdx
		 *   E_OVERLAP if sector overlaps with existing sector
		 * 'second' insertion index if location is valid
		 */
		std::pair<Track::RetCode, size_t>
		findSectorInsertionIdx(
			size_t entryIdx,
			size_t exitIdx);
		
		/**
		 * @param[in] name
		 * the sector's name
		 * 
		 * @param[in] entryIdx
		 * the sector's entry point on the path
		 * 
		 * @param[in] exitIdx
		 * the sector's exit point on the path
		 * 
		 * @return
		 * 'first' is the return code
		 *   SUCCESS if add was successful
		 *   E_SECTOR_NO_WIDTH if entry and exit are equal
		 *   E_EXIT_BEFORE_ENTRY if exitIdx falls before entryIdx
		 *   E_OVERLAP if sector overlaps with existing sector
		 * 'second' is the insertion index if successful
		 */
		std::pair<Track::RetCode, size_t>
		addSector(
			const std::string &name,
			const size_t entryIdx,
			const size_t exitIdx);

		void
		removeSector(
			const size_t idx);

		void
		setSectorName(
			const size_t idx,
			const std::string &name);

		std::shared_ptr<const TrackSector>
		getSector(
			const size_t idx) const;

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
			const cv::Vec2d &p,
			const double width_meters) const;

		std::pair<bool, cv::Vec2d>
		findClosestPoint(
			const cv::Vec2d &p) const;

		std::tuple<bool, cv::Vec2d, size_t>
		findClosestPointWithIdx(
			const cv::Vec2d &p) const;

		std::tuple<bool, cv::Vec2d, size_t>
		findClosestPointWithIdx(
			const cv::Vec2d &p,
			const size_t initialIdx,
			const std::pair<size_t,size_t> &window) const;

		bool
		getSortedPathObjects(
			std::vector<const TrackPathObject *> &objs) const;

		// YAML encode/decode
		YAML::Node
		encode() const;

		bool
		decode(
			const YAML::Node& node);

	protected:
        bool
        subclassApplyModifications(
        	bool unnecessaryIsOkay) final;

        bool
        subclassSaveModifications(
        	bool unnecessaryIsOkay) final;

	private:
		TrackGate start_;
		TrackGate finish_;
		std::vector<std::shared_ptr<TrackSector>> sectors_;
		// list of lat/lon points making up the track's path
		std::vector<cv::Vec2d> path_;

	};

	TrackPtr
	makeTrackFromTelemetry(
		TelemetrySourcePtr tSrc);

}

// overrides for yaml config encode/decode
namespace YAML
{

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