#include <csignal>
#include <getopt.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <unistd.h>// sleep
#include <time.h>

#include <GoProTelem/GoProTelem.h>
#include <MultiStopwatch.h>

const char *PROG_NAME = "gopro_overlay";
bool stop_app = false;

struct prog_options
{
};

void
handle_sigint(
	int signal)
{
	stop_app = true;
}

void
display_usage()
{
	printf("usage: %s videofile [options]\n",PROG_NAME);
	printf(" -h,--help        : display this menu\n");
}

uint64_t
get_ticks_usec()
{
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000UL + ts.tv_nsec / 1000UL;
}

void
parse_args(
	int argc,
	char *argv[],
	prog_options &opts)
{
	static struct option long_options[] =
	{
		{0, 0, 0, 0}
	};

	while (true)
	{
		int option_index = 0;
		int c = getopt_long(
			argc,
			argv,
			"h",
			long_options,
			&option_index);

		// detect the end of the options
		if (c == -1)
		{
			break;
		}

		switch (c)
		{
			case 0:
				// flag setting
				break;
			case 'h':
			case '?':
			default:
				display_usage();
				exit(0);
				break;
		}
	}
}

void
drawG_Circle(
	cv::Mat &image,
	cv::Point gCenter,
	int gRadius,
	const std::vector<cv::Point_<double>> &gPoints)
{
	// draw outer circle
	cv::circle(image,gCenter,gRadius,CV_RGB(2, 155, 250),4);

	// draw trail
	for (size_t i=0; i<gPoints.size(); i++)
	{
		bool isLast = i == gPoints.size()-1;
		auto color = (isLast ? CV_RGB(2, 155, 250) : CV_RGB(247, 162, 2));
		int dotRadius = (isLast ? 10 : 3);
		int thickness = (isLast ? 3 : 3);
		const auto &point = gPoints.at(i);

		auto drawPoint = cv::Point(
			(point.x / -9.8) * gRadius + gCenter.x,
			(point.y / 9.8) * gRadius + gCenter.y);
		cv::circle(image,drawPoint,dotRadius,color,thickness);
	}
}

void
addG_CirclePoint(
	std::vector<cv::Point_<double>> &gPoints,
	size_t length,
	const gpt::AcclSample &acclSample)
{
	auto newPoint = cv::Point_<double>(acclSample.x,acclSample.z);
	if (length > 1 && gPoints.size() == length)
	{
		for (size_t i=0; i<gPoints.size()-1; i++)
		{
			gPoints[i] = gPoints[i+1];
		}
		gPoints[length-1] = newPoint;
	}
	else
	{
		gPoints.push_back(newPoint);
	}
}

int main(int argc, char *argv[])
{
	signal(SIGINT, handle_sigint);// ctrl+c to stop application

	prog_options opts;
	parse_args(argc,argv,opts);

	printf("openening %s\n", argv[1]);
	cv::VideoCapture vcap(argv[1]);
	if ( ! vcap.isOpened())
	{
		printf("No image data\n");
		return -1;
	}

	gpt::MP4_Source mp4;
	mp4.open(argv[1]);
	auto telemData = gpt::getCombinedSamples(mp4);

	MultiStopwatch msw;
	msw.enabled(false);
	auto SW_RECORD_READ = msw.addRecord("ImageRead",10000);
	auto SW_RECORD_RESIZE = msw.addRecord("ImageResize",10000);
	auto SW_RECORD_SHOW = msw.addRecord("ImageShow",10000);
	auto SW_RECORD_WAITKEY = msw.addRecord("WaitKey",10000);

	const auto OUT_VIDEO_SIZE = cv::Size(1280,720);
	const auto TEXT_FONT = cv::FONT_HERSHEY_DUPLEX;
	const auto TEXT_COLOR = CV_RGB(118, 185, 0);
	const double G_CIRCLE_HISTORY_SEC = 1.0;
	const int G_CIRCLE_RADIUS = 100;
	const auto G_CIRCLE_CENTER = cv::Point(
		OUT_VIDEO_SIZE.width - G_CIRCLE_RADIUS - 50,
		OUT_VIDEO_SIZE.height - G_CIRCLE_RADIUS - 50);
	double frames_count = vcap.get(cv::CAP_PROP_FRAME_COUNT);
	double fps = vcap.get(cv::CAP_PROP_FPS);
	double frame_time_sec = 1.0 / fps;
	double frame_time_usec = 1.0e6 / fps;
	int gPointSize = G_CIRCLE_HISTORY_SEC * fps;
	int frame_time_ms = std::round(1.0e3 / fps);
	printf("frames_count = %f\n", frames_count);
	printf("fps = %f\n", fps);
	printf("frame_time_ms = %d\n", frame_time_ms);

	cv::Mat frame;
	cv::Mat out_frame;
	char frame_idx_str[1024];
	char frame_time_str[1024];
	char accl_str[1024];
	double time_offset_sec = 0.0;
	std::vector<cv::Point_<double>> gPoints;
	uint64_t prev_frame_start_usec = 0;
	int64_t frame_time_err_usec = 0;// (+) means measured frame time was longer than targeted FPS
	for (size_t frame_idx=0; ! stop_app && frame_idx<frames_count; frame_idx++)
	{
		uint64_t frame_start_usec = get_ticks_usec();
		if (prev_frame_start_usec != 0)
		{
			int64_t meas_frame_time_usec = frame_start_usec - prev_frame_start_usec;
			frame_time_err_usec = meas_frame_time_usec - frame_time_usec;
			printf("frame_time_err_usec = %ld\n",frame_time_err_usec);
		}

		// Capture frame-by-frame
		msw.start(SW_RECORD_READ);
		bool read_okay = vcap.read(frame);
		msw.stop(SW_RECORD_READ);
		if (read_okay)
		{
			msw.start(SW_RECORD_RESIZE);
			cv::resize(frame,out_frame,OUT_VIDEO_SIZE);
			msw.stop(SW_RECORD_RESIZE);

			sprintf(frame_idx_str,"frame_idx: %ld",frame_idx);
			cv::putText(
				out_frame, //target image
				frame_idx_str, //text
				cv::Point(10, 30), //top-left position
				TEXT_FONT,// font face
				1.0,// font scale
				TEXT_COLOR, //font color
				1);// thickness

			sprintf(frame_time_str,"time_offset: %0.3fs",time_offset_sec);
			cv::putText(
				out_frame, //target image
				frame_time_str, //text
				cv::Point(10, 30 * 2), //top-left position
				TEXT_FONT,// font face
				1.0,// font scale
				TEXT_COLOR, //font color
				1);// thickness

			auto &telemSamp = telemData.at(frame_idx);
			printf("%s\n",telemSamp.toString().c_str());
			sprintf(accl_str,"accl: %s",telemSamp.accl.toString().c_str());
			cv::putText(
				out_frame, //target image
				accl_str, //text
				cv::Point(10, 30 * 3), //top-left position
				TEXT_FONT,// font face
				1.0,// font scale
				TEXT_COLOR, //font color
				1);// thickness

			addG_CirclePoint(gPoints,gPointSize,telemSamp.accl);
			drawG_Circle(out_frame,G_CIRCLE_CENTER,G_CIRCLE_RADIUS,gPoints);

			// Display the resulting frame
			msw.start(SW_RECORD_SHOW);
			cv::imshow("Video", out_frame);
			msw.stop(SW_RECORD_SHOW);


			double usec_of_processing = get_ticks_usec() - frame_start_usec;
			int ms_to_wait = std::round((frame_time_usec - usec_of_processing) / 1000.0);
			if (ms_to_wait <= 0) {
				// processing is so slow that it ate up all out frame time
				// need to wait at least a little for the OpenCV to do it's drawing
				ms_to_wait = 1;
			}
			if (ms_to_wait > 0) {
				// Press Q on keyboard to exit
				msw.start(SW_RECORD_WAITKEY);
				auto keycode = cv::waitKey(ms_to_wait);
				msw.stop(SW_RECORD_WAITKEY);
				if (keycode & 0xFF == 'q')
				{
					printf("Quit video playback!\n");
					break;
				}
			}

			time_offset_sec += frame_time_sec;
			prev_frame_start_usec = frame_start_usec;
		}
		else
		{
			// Break the loop
			// i'm seeing GoPro videos do this about 1s into the clip.
			// i wonder if they're encoder some metadata or something.
			printf("frame %ld is bad?\n", frame_idx);
		}
	}

	if (msw.enabled())
	{
		printf("%s\n",msw.getSummary().c_str());
	}

	return 0;
}