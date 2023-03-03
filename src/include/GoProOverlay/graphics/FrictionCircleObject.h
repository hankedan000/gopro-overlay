#pragma once

#include "GoProOverlay/graphics/RenderedObject.h"

namespace gpo
{
	class FrictionCircleObject : public RenderedObject
	{
	public:
		FrictionCircleObject();

		virtual
		DataSourceRequirements
		dataSourceRequirements() const override;

		void
		setTailLength(
			size_t tailLength);

		size_t
		getTailLength() const;

		void
		setBorderColor(
			cv::Scalar rgbColor);

		cv::Scalar
		getBorderColor() const;

		void
		setTailColor(
			cv::Scalar rgbColor);

		cv::Scalar
		getTailColor() const;

		void
		setCurrentDotColor(
			cv::Scalar rgbColor);

		cv::Scalar
		getCurrentDotColor() const;

	protected:
		virtual
		void
		subRender() override;

		virtual
		YAML::Node
		subEncode() const override;

		virtual
		bool
		subDecode(
			const YAML::Node& node) override;

	private:
		void
		redrawOutline();

	private:
		// map outline
		cv::UMat outlineImg_;

		size_t tailLength_;

		int radius_px_;
		int margin_px_;
		cv::Point center_;

		cv::Scalar borderColor_;
		cv::Scalar tailColor_;
		cv::Scalar currentDotColor_;

	};
}