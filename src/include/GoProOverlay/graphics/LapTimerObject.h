#pragma once

#include "GoProOverlay/graphics/RenderedObject.h"

namespace gpo
{
	class LapTimerObject : public RenderedObject
	{
	public:
		LapTimerObject();

		virtual
		DataSourceRequirements
		dataSourceRequirements() const override;

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
		// background image
		cv::UMat bgImg_;

		cv::Scalar textColor_;

		double lapTime_;

	};
}