#include "GoProOverlay/graphics/LapTimerObject.h"

namespace gpo
{

	const int LAPTIMER_RENDERED_WIDTH = 600;
	const int LAPTIMER_RENDERED_HEIGHT = 200;

	LapTimerObject::LapTimerObject()
	 : RenderedObject(LAPTIMER_RENDERED_WIDTH,LAPTIMER_RENDERED_HEIGHT)
	 , bgImg_(LAPTIMER_RENDERED_HEIGHT,LAPTIMER_RENDERED_WIDTH,CV_8UC4,RGBA_COLOR(0,0,0,0))
	 , textColor_(RGBA_COLOR(255,255,255,255))
	 , startIdx_(-1)
	 , finishIdx_(-1)
	 , isLapFinished_(false)
	{
	}

	DataSourceRequirements
	LapTimerObject::dataSourceRequirements() const
	{
		return DataSourceRequirements(0,1,0);
	}

	bool
	LapTimerObject::init(
		int lapStartIdx,
		int lapFinishIdx)
	{
		startIdx_ = lapStartIdx;
		finishIdx_ = lapFinishIdx;
		return true;
	}

	void
	LapTimerObject::render(
		cv::Mat &intoImg,
		int originX, int originY,
		cv::Size renderSize)
	{
		bgImg_.copyTo(outImg_);
		if ( ! requirementsMet())
		{
			return;
		}

		auto telemSrc = tSources_.front();

		// constrain the start/finish indices to within the samples vector
		size_t startIdx = startIdx_;
		if (startIdx < 0)
		{
			startIdx = 0;
		}
		size_t finishIdx = finishIdx_;
		if (finishIdx < 0 || finishIdx >= telemSrc->size())
		{
			finishIdx = telemSrc->size() - 1;
		}

		double lapTime = 0.0;
		const auto &startSamp = telemSrc->at(startIdx);
		const auto &finishSamp = telemSrc->at(finishIdx);
		if (isLapFinished_)
		{
			lapTime = finishSamp.gpSamp.t_offset - startSamp.gpSamp.t_offset;
		}
		else if ( ! isLapFinished_ && telemSrc->seekedIdx() >= finishIdx)
		{
			isLapFinished_ = true;
			lapTime = finishSamp.gpSamp.t_offset - startSamp.gpSamp.t_offset;
		}
		else if (telemSrc->seekedIdx() > startIdx_)
		{
			lapTime = telemSrc->at(telemSrc->seekedIdx()).gpSamp.t_offset - startSamp.gpSamp.t_offset;
		}

		char lapTimeStr[1024];
		sprintf(lapTimeStr,"Lap Time: %0.3fs", lapTime);
		cv::putText(
			outImg_, // target image
			lapTimeStr, // text
			cv::Point(0,60),
			cv::FONT_HERSHEY_DUPLEX,// font face
			2.0,// font scale
			textColor_, //font color
			2);// thickness

		// let base class perform its own rendering too
		RenderedObject::render(intoImg,originX,originY,renderSize);
	}

}