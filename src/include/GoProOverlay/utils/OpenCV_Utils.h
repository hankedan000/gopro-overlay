#pragma once

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>

namespace cv
{
    
	/**
	 * Draws a rectangle with rounded corners, the parameters are the same as in the OpenCV function @see rectangle();
	 * @param cornerRadius A positive int value defining the radius of the round corners.
	 * @author K
	 */
	void
	rounded_rectangle(
		cv::UMat& src,
		cv::Point topLeft, cv::Point bottomRight,
		const cv::Scalar color,
		const int thickness,
		const int lineType ,
		const int cornerRadius);

}