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

		void
		addSource(
			const TelemetrySource &tSrc);

		TelemetrySource
		getSource(
			size_t idx) const;

		size_t
		sourceCount() const;

	protected:
		std::vector<TelemetrySource> sources_;

	};
}