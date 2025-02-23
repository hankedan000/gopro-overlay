#pragma once

#include <opencv2/core/matx.hpp> // for cv::Vec2d

namespace utils
{
	// Given three colinear points p, q, r, the function checks if 
	// point q lies on line segment 'pr' 
	bool
	onSegment(
		cv::Vec2d p,
		cv::Vec2d q,
		cv::Vec2d r);

	// To find orientation of ordered triplet (p, q, r). 
	// The function returns following values 
	// 0 --> p, q and r are colinear 
	// 1 --> Clockwise 
	// 2 --> Counterclockwise 
	int
	orientation(
		cv::Vec2d p,
		cv::Vec2d q,
		cv::Vec2d r);

	// The main function that returns true if line segment 'p1q1' 
	// and 'p2q2' intersect. 
	bool
	doIntersect(
		cv::Vec2d p1, cv::Vec2d q1,
		cv::Vec2d p2, cv::Vec2d q2);
}
