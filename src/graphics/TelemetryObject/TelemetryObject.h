#pragma once

#include <vector>

#include "RenderedObject.h"
#include "TelemetrySource.h"

namespace gpo
{
	class TelemetryObject : public RenderedObject
	{
	public:
		TelemetryObject(
			int width,
			int height);

		virtual
		void
		addSource(
			const TelemetrySourcePtr &tSrc);

		TelemetrySourcePtr
		getSource(
			size_t idx) const;

		size_t
		sourceCount() const;

	protected:
		std::vector<TelemetrySourcePtr> sources_;

	};
}