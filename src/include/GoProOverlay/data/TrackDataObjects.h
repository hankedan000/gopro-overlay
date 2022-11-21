#pragma once

#include <opencv2/opencv.hpp>
#include <tuple>
#include <vector>

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

	private:
		cv::Vec2d a_;
		cv::Vec2d b_;
	};

	class Sector
	{
	public:
		Sector(
			const std::string &name,
			const DetectionGate &entry,
			const DetectionGate &exit);

		const std::string &
		name() const;

		const DetectionGate &
		entry() const;
		
		const DetectionGate &
		exit() const;

	private:
		std::string name_;
		DetectionGate entry_;
		DetectionGate exit_;

	};

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
			Sector *s);

		bool
		removeSector(
			size_t idx);

		Sector *
		getSector(
			size_t idx);

		size_t
		sectorCount() const;

		size_t
		pathCount() const;

		cv::Vec2d
		getPathPoint(
			size_t idx);

		DetectionGate
		getNearestDetectionGate(
			cv::Vec2d p,
			double width_meters);

		std::pair<bool,cv::Vec2d>
		findClosestPoint(
			cv::Vec2d p);

		std::tuple<bool,cv::Vec2d, size_t>
		findClosestPointWithIdx(
			cv::Vec2d p);

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