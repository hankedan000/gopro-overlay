#include "GoProOverlay/utils/OpenCV_Utils.h"

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
		const int cornerRadius)
	{
		/* corners:
		* p1 - p2
		* |     |
		* p4 - p3
		*/
		cv::Point p1 = topLeft;
		cv::Point p2 = cv::Point(bottomRight.x, topLeft.y);
		cv::Point p3 = bottomRight;
		cv::Point p4 = cv::Point(topLeft.x, bottomRight.y);
		
		if (thickness < 0)
		{
			// big rect
			cv::Point top_left_main_rect(int(p1.x + cornerRadius), int(p1.y));
			cv::Point bottom_right_main_rect(int(p3.x - cornerRadius), int(p3.y));

			cv::Point top_left_rect_left(p1.x, p1.y + cornerRadius);
			cv::Point bottom_right_rect_left(p4.x + cornerRadius, p4.y - cornerRadius);

			cv::Point top_left_rect_right(p2.x - cornerRadius, p2.y + cornerRadius);
			cv::Point bottom_right_rect_right(p3.x, p3.y - cornerRadius);

			cv::rectangle(src, top_left_main_rect, bottom_right_main_rect, color, thickness);
			cv::rectangle(src, top_left_rect_left, bottom_right_rect_left, color, thickness);
			cv::rectangle(src, top_left_rect_right, bottom_right_rect_right, color, thickness);
		}

		// draw straight lines
		line(src, cv::Point(p1.x + cornerRadius, p1.y), cv::Point(p2.x - cornerRadius, p2.y), color, std::abs(thickness), lineType);
		line(src, cv::Point(p2.x, p2.y + cornerRadius), cv::Point(p3.x, p3.y - cornerRadius), color, std::abs(thickness), lineType);
		line(src, cv::Point(p4.x + cornerRadius, p4.y), cv::Point(p3.x - cornerRadius, p3.y), color, std::abs(thickness), lineType);
		line(src, cv::Point(p1.x, p1.y + cornerRadius), cv::Point(p4.x, p4.y - cornerRadius), color, std::abs(thickness), lineType);
		
		// draw arcs
		ellipse(src, p1 + cv::Point(+cornerRadius, +cornerRadius), cv::Size(cornerRadius, cornerRadius), 180.0, 0, 90, color, thickness, lineType);
		ellipse(src, p2 + cv::Point(-cornerRadius, +cornerRadius), cv::Size(cornerRadius, cornerRadius), 270.0, 0, 90, color, thickness, lineType);
		ellipse(src, p3 + cv::Point(-cornerRadius, -cornerRadius), cv::Size(cornerRadius, cornerRadius),   0.0, 0, 90, color, thickness, lineType);
		ellipse(src, p4 + cv::Point(+cornerRadius, -cornerRadius), cv::Size(cornerRadius, cornerRadius),  90.0, 0, 90, color, thickness, lineType);
	}
}