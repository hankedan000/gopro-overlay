#pragma once

#include "GoProOverlay/graphics/RenderedObject.h"

namespace gpo
{
	class FrictionCircleObject : public RenderedObject
	{
	public:
		FrictionCircleObject();

		virtual
		std::string
		typeName() const override;

		virtual
		DataSourceRequirements
		dataSourceRequirements() const override;

		void
		setTailLength(
			size_t tailLength);

		virtual
		void
		render(
			cv::Mat &intoImg,
			int originX, int originY,
			cv::Size renderSize) override;

	protected:
		virtual
		YAML::Node
		subEncode() const override;

		virtual
		bool
		subDecode(
			const YAML::Node& node) override;

	private:
		// map outline
		cv::Mat outlineImg_;

		size_t tailLength_;

		int radius_px_;
		int margin_px_;
		cv::Point center_;

		cv::Scalar borderColor_;
		cv::Scalar currentDotColor_;

	};
}