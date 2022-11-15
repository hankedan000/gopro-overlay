#include <csignal>
#include <getopt.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <unistd.h>// sleep
#include <time.h>

#include <GoProTelem/GoProTelem.h>

#include "FrictionCircleObject.h"
#include "TrackMapObject.h"
#include "tqdm.h"

const char *PROG_NAME = "gopro_overlay";
bool stop_app = false;

struct ProgOptions
{
};

void
handleSIGINT(
	int signal)
{
	stop_app = true;
}

void
displayUsage()
{
	printf("usage: %s videofile [options]\n",PROG_NAME);
	printf(" -h,--help        : display this menu\n");
}

uint64_t
getTicks_usec()
{
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000UL + ts.tv_nsec / 1000UL;
}

void
parseArgs(
	int argc,
	char *argv[],
	ProgOptions &opts)
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
				displayUsage();
				exit(0);
				break;
		}
	}
}

int main(int argc, char *argv[])
{
	signal(SIGINT, handleSIGINT);// ctrl+c to stop application

	ProgOptions opts;
	parseArgs(argc,argv,opts);

	printf("openening %s\n", argv[1]);
	cv::VideoCapture vCap(argv[1]);
	if ( ! vCap.isOpened())
	{
		printf("No image data\n");
		return -1;
	}

	gpt::MP4_Source mp4;
	mp4.open(argv[1]);
	auto telemData = gpt::getCombinedSamples(mp4);

	const bool SHOW_LIVE_VIDEO = true;
	const bool RENDER_DEBUG_INFO = false;
	const auto OUT_VIDEO_SIZE = cv::Size(1280,720);
	const auto TEXT_FONT = cv::FONT_HERSHEY_DUPLEX;
	const auto TEXT_COLOR = CV_RGB(0, 255, 0);
	const double F_CIRCLE_HISTORY_SEC = 1.0;
	const int F_CIRCLE_RADIUS = 100;
	double frameCount = vCap.get(cv::CAP_PROP_FRAME_COUNT);
	double fps = vCap.get(cv::CAP_PROP_FPS);
	double frameTime_sec = 1.0 / fps;
	double frameTime_usec = 1.0e6 / fps;
	int fcTailLength = F_CIRCLE_HISTORY_SEC * fps;
	int frameTime_ms = std::round(1.0e3 / fps);
	printf("frameCount = %f\n", frameCount);
	printf("fps = %f\n", fps);
	printf("frameTime_ms = %d\n", frameTime_ms);

	cv::Mat frame;
	cv::Mat outFrame;
	gpo::TrackMapObject trackMap(400,400);
	trackMap.initMap(telemData);
	gpo::FrictionCircleObject frictionCircle(F_CIRCLE_RADIUS,20);
	frictionCircle.init();
	cv::VideoWriter vWriter(
		"render.mp4",
		cv::VideoWriter::fourcc('M','P','4','V'),
		fps,
		OUT_VIDEO_SIZE,
		true);
	char tmpStr[1024];
	double timeOffset_sec = 0.0;
	tqdm bar;// for render progress
	uint64_t prevFrameStart_usec = 0;
	int64_t frameTimeErr_usec = 0;// (+) means measured frame time was longer than targeted FPS
	size_t initFrameIdx = 0;
	vCap.set(cv::CAP_PROP_POS_FRAMES, initFrameIdx);
	for (size_t frameIdx=initFrameIdx; ! stop_app && frameIdx<frameCount; frameIdx++)
	{
		uint64_t frameStart_usec = getTicks_usec();
		if (prevFrameStart_usec != 0)
		{
			int64_t meas_frameTime_usec = frameStart_usec - prevFrameStart_usec;
			frameTimeErr_usec = meas_frameTime_usec - frameTime_usec;
			// printf("frameTimeErr_usec = %ld\n",frameTimeErr_usec);
		}

		// show render progress
		if ( ! SHOW_LIVE_VIDEO)
		{
			auto total = frameCount - initFrameIdx;
			auto curr = frameIdx - initFrameIdx;
			bar.progress(curr,total);
		}

		auto &telemSamp = telemData.at(frameIdx);
		// printf("%s\n",telemSamp.toString().c_str());

		// Capture frame-by-frame
		if (vCap.read(frame))
		{
			cv::resize(frame,outFrame,OUT_VIDEO_SIZE);

			if (RENDER_DEBUG_INFO)
			{
				sprintf(tmpStr,"frameIdx: %ld",frameIdx);
				cv::putText(
					outFrame, //target image
					tmpStr, //text
					cv::Point(10, 30), //top-left position
					TEXT_FONT,// font face
					1.0,// font scale
					TEXT_COLOR, //font color
					1);// thickness

				sprintf(tmpStr,"time_offset: %0.3fs",timeOffset_sec);
				cv::putText(
					outFrame, //target image
					tmpStr, //text
					cv::Point(10, 30 * 2), //top-left position
					TEXT_FONT,// font face
					1.0,// font scale
					TEXT_COLOR, //font color
					1);// thickness

				sprintf(tmpStr,"accl: %s",telemSamp.accl.toString().c_str());
				cv::putText(
					outFrame, //target image
					tmpStr, //text
					cv::Point(10, 30 * 3), //top-left position
					TEXT_FONT,// font face
					1.0,// font scale
					TEXT_COLOR, //font color
					1);// thickness
			}

			int speedMPH = round(telemSamp.gps.speed2D * 2.23694);// m/s to mph
			sprintf(tmpStr,"%2dmph",speedMPH);
			cv::putText(
				outFrame, // target image
				tmpStr, // text
				cv::Point(10, OUT_VIDEO_SIZE.height - 30), // bottom-left position
				TEXT_FONT,// font face
				2.0,// font scale
				CV_RGB(2,155,250), // font color
				2);// thickness

			frictionCircle.updateTail(telemData,frameIdx,fcTailLength);
			frictionCircle.render(
				outFrame,
				outFrame.cols - F_CIRCLE_RADIUS * 2 - 50,
				outFrame.rows - F_CIRCLE_RADIUS * 2 - 50);

			trackMap.setLocation(telemSamp.gps.coord);
			trackMap.render(outFrame,outFrame.cols - 400,0);

			// write frame to video file
			vWriter.write(outFrame);

			// Display the frame live
			if (SHOW_LIVE_VIDEO)
			{
				cv::imshow("Video", outFrame);
				double processingTime_usec = getTicks_usec() - frameStart_usec;
				int waitTime_ms = std::round((frameTime_usec - processingTime_usec) / 1000.0);
				if (waitTime_ms <= 0)
				{
					// processing is so slow that it ate up all out frame time
					// need to wait at least a little for the OpenCV to do it's drawing
					waitTime_ms = 1;
				}
				if (waitTime_ms > 0)
				{
					// Press Q on keyboard to exit
					auto keycode = cv::waitKey(waitTime_ms);
					if (keycode & 0xFF == 'q')
					{
						printf("Quit video playback!\n");
						break;
					}
				}
			}

			timeOffset_sec += frameTime_sec;
			prevFrameStart_usec = frameStart_usec;
		}
		else
		{
			// i'm seeing GoPro videos do this about 1s into the clip.
			// i wonder if they're encoder some metadata or something.
			printf("frame %ld is bad?\n", frameIdx);
		}
	}

	if ( ! SHOW_LIVE_VIDEO)
	{
		bar.finish();
	}

	vWriter.release();// close video file

	return 0;
}