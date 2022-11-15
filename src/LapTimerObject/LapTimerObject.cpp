#include "LapTimerObject.h"

namespace gpo
{

	LapTimerObject::LapTimerObject()
	 : RenderedObject(400,200)
	 , bgImg_(200,400,CV_8UC4,RGBA_COLOR(0,0,0,0))
	 , textColor_(RGBA_COLOR(255,255,255,255))
	 , startIdx_(-1)
	 , finishIdx_(-1)
	 , isLapFinished_(false)
	{
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
	LapTimerObject::updateTimer(
		const std::vector<gpt::CombinedSample> &samples,
		size_t currentIndex)
	{
		bgImg_.copyTo(outImg_);

		// constrain the start/finish indices to within the samples vector
		size_t startIdx = startIdx_;
		if (startIdx < 0)
		{
			startIdx = 0;
		}
		size_t finishIdx = finishIdx_;
		if (finishIdx < 0 || finishIdx >= samples.size())
		{
			finishIdx = samples.size() - 1;
		}

		double lapTime = 0.0;
		if (isLapFinished_)
		{
			lapTime = samples.at(finishIdx).t_offset - samples.at(startIdx).t_offset;
		}
		else if ( ! isLapFinished_ && currentIndex >= finishIdx)
		{
			isLapFinished_ = true;
			lapTime = samples.at(finishIdx).t_offset - samples.at(startIdx).t_offset;
		}
		else if (currentIndex > startIdx_)
		{
			lapTime = samples.at(currentIndex).t_offset - samples.at(startIdx_).t_offset;
		}

		char lapTimeStr[1024];
		sprintf(lapTimeStr,"Lap Time: %0.3fs", lapTime);
		cv::putText(
			outImg_, // target image
			lapTimeStr, // text
			cv::Point(0,30),
			cv::FONT_HERSHEY_DUPLEX,// font face
			1.0,// font scale
			textColor_, //font color
			1);// thickness
	}

}