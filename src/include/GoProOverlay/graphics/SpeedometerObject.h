#pragma once

#include "GoProOverlay/graphics/RenderedObject.h"

namespace gpo
{
	class SpeedometerObject : public RenderedObject
	{
	public:
		SpeedometerObject();

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

	};
}