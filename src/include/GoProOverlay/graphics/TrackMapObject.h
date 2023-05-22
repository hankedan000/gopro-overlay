#pragma once

#include <GoProTelem/SampleTypes.h>

#include "GoProOverlay/graphics/RenderedObject.h"

namespace gpo
{
	class TrackMapObject : public RenderedObject
	{
	public:
		TrackMapObject();

		virtual
		DataSourceRequirements
		dataSourceRequirements() const override;

		void
		setDotColor(
			size_t sourceIdx,
			cv::Scalar color);

	protected:
		virtual
		void
		subRender() override;

		// callback from RenderedObject class when all source requirements are met
		virtual
		void
		sourcesValid() override;

		virtual
		YAML::Node
		subEncode() const override;

		virtual
		bool
		subDecode(
			const YAML::Node& node) override;

	private:
		cv::Point
		coordToPoint(
			const gpt::CoordLL &coord);

	private:
		const int PX_MARGIN = 20;

		// map outline
		cv::UMat outlineImg_;

		// coordinates of upper-left and lower-right corners
		gpt::CoordLL ulCoord_;
		gpt::CoordLL lrCoord_;

		double pxPerDeg_;

		const double DEFAULT_TRACK_THICKNESS_RATIO = 2.0 / 300.0;// 'line thickness' over 'rendered trackmap width'
		int trackThickness_px_;

		const double DEFAULT_DOT_RADIUS_RATIO = 5.0 / 300.0;// 'dot radius' over 'rendered trackmap width'
		int dotRadius_px_;

		std::vector<cv::Scalar> dotColors_;

	};
}