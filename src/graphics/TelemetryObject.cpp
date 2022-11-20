#include "GoProOverlay/graphics/TelemetryObject.h"

namespace gpo
{
	TelemetryObject::TelemetryObject(
		int width,
		int height)
	 : RenderedObject(width,height)
	 , sources_()
	{
	}

	void
	TelemetryObject::addSource(
		const TelemetrySourcePtr &tSrc)
	{
		sources_.push_back(tSrc);
	}

	TelemetrySourcePtr
	TelemetryObject::getSource(
		size_t idx) const
	{
		return sources_.at(idx);
	}

	size_t
	TelemetryObject::sourceCount() const
	{
		return sources_.size();
	}
}