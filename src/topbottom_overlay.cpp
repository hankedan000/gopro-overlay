#include <csignal>
#include <getopt.h>
#include <iostream>
#include <unistd.h>// sleep
#include <time.h>

#include "DataFactory.h"
#include "FrictionCircleObject.h"
#include "LapTimerObject.h"
#include "SpeedometerObject.h"
#include "TelemetryPrintoutObject.h"
#include "TrackMapObject.h"
#include "tqdm.h"
#include "VideoObject.h"

const char *PROG_NAME = "topbottom_overlay";
bool stop_app = false;

struct ProgOptions
{
	std::string topFile;
	std::string bottomFile;
	std::string outputFile = "render_topbottom.mp4";
	int showPreview = 0;
	int renderDebugInfo = 0;
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
	printf("usage: %s -t <video_file> -b <video_file> [options]\n",PROG_NAME);
	printf(" -t,--topFile        : the input video file for top view\n");
	printf(" -b,--bottomFile     : the input video file for bottom view\n");
	printf(" -o,--outputFile     : the render output video file\n");
	printf(" --showPreview       : display live render preview\n");
	printf(" --renderDebugInfo   : render debug information to video\n");
	printf(" -h,--help           : display this menu\n");
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
		{"topFile"            , required_argument , 0                      , 't' },
		{"bottomFile"         , required_argument , 0                      , 'b' },
		{"showPreview"        , no_argument       , &opts.showPreview      , 1   },
		{"renderDebugInfo"    , no_argument       , &opts.renderDebugInfo  , 1   },
		{"help"               , no_argument       , 0                      , 'h' },
		{0, 0, 0, 0}
	};

	while (true)
	{
		int option_index = 0;
		int c = getopt_long(
			argc,
			argv,
			"ht:b:o:",
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
			case 't':
				opts.topFile = optarg;
				break;
			case 'b':
				opts.bottomFile = optarg;
				break;
			case 'o':
				opts.outputFile = optarg;
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

	if (opts.topFile.empty())
	{
		printf("no top video file provided.\n");
		displayUsage();
		exit(0);
	}
	else if (opts.bottomFile.empty())
	{
		printf("no bottom video file provided.\n");
		displayUsage();
		exit(0);
	}

	printf("opening %s\n", opts.topFile.c_str());
	gpo::Data topData;
	if ( ! gpo::DataFactory::loadData(opts.topFile,topData))
	{
		printf("No top video data\n");
		return -1;
	}
	printf("opening %s\n", opts.bottomFile.c_str());
	gpo::Data botData;
	if ( ! gpo::DataFactory::loadData(opts.bottomFile,botData))
	{
		printf("No bottom video data\n");
		return -1;
	}

	const auto RENDERED_VIDEO_SIZE = topData.videoSrc->frameSize();
	const auto PREVIEW_VIDEO_SIZE = cv::Size(1280,720);
	const double F_CIRCLE_HISTORY_SEC = 1.0;
	double frameCount = topData.videoSrc->frameCount();
	double fps = topData.videoSrc->fps();
	double frameTime_sec = 1.0 / fps;
	double frameTime_usec = 1.0e6 / fps;
	int fcTailLength = F_CIRCLE_HISTORY_SEC * fps;
	int frameTime_ms = std::round(1.0e3 / fps);

	cv::Mat rFrame(RENDERED_VIDEO_SIZE,CV_8UC3);// rendered frame
	cv::Mat pFrame;// preview frame

	gpo::VideoObject topVideoObject(topData.videoSrc);
	cv::Size topVideoSize = topVideoObject.getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 2.0);
	gpo::VideoObject botVideoObject(botData.videoSrc);
	cv::Size botVideoSize = botVideoObject.getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 2.0);

	gpo::TrackMapObject trackMap;
	cv::Size tmRenderSize = trackMap.getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 3.0);
	trackMap.addSource(topData.telemSrc);
	trackMap.addSource(botData.telemSrc);
	trackMap.initMap();

	gpo::FrictionCircleObject topFC;
	cv::Size topFC_RenderSize = topFC.getScaledSizeFromTargetHeight(topVideoSize.height / 2.0);
	topFC.setTailLength(fcTailLength);
	topFC.addSource(topData.telemSrc);
	gpo::FrictionCircleObject botFC;
	cv::Size botFC_RenderSize = botFC.getScaledSizeFromTargetHeight(botVideoSize.height / 2.0);
	botFC.setTailLength(fcTailLength);
	botFC.addSource(botData.telemSrc);

	gpo::LapTimerObject lapTimer;
	cv::Size ltRenderSize = lapTimer.getScaledSizeFromTargetHeight(RENDERED_VIDEO_SIZE.height / 10.0);
	lapTimer.init(0,topData.telemSrc->size()-1);
	lapTimer.addSource(topData.telemSrc);

	gpo::TelemetryPrintoutObject topPrintoutObject;
	topPrintoutObject.addSource(topData.telemSrc);
	topPrintoutObject.setVisible(opts.renderDebugInfo);
	gpo::TelemetryPrintoutObject botPrintoutObject;
	botPrintoutObject.addSource(botData.telemSrc);
	botPrintoutObject.setVisible(opts.renderDebugInfo);

	gpo::SpeedometerObject topSpeedoObject;
	cv::Size topSpeedoRenderSize = topSpeedoObject.getScaledSizeFromTargetHeight(topVideoSize.height / 4.0);
	topSpeedoObject.addSource(topData.telemSrc);
	gpo::SpeedometerObject botSpeedoObject;
	cv::Size botSpeedoRenderSize = botSpeedoObject.getScaledSizeFromTargetHeight(botVideoSize.height / 4.0);
	botSpeedoObject.addSource(botData.telemSrc);

	cv::VideoWriter vWriter(
		opts.outputFile,
		cv::VideoWriter::fourcc('M','P','4','V'),
		fps,
		RENDERED_VIDEO_SIZE,
		true);
	char tmpStr[1024];
	double timeOffset_sec = 0.0;
	tqdm bar;// for render progress
	uint64_t prevFrameStart_usec = 0;
	int64_t frameTimeErr_usec = 0;// (+) means measured frame time was longer than targeted FPS
	size_t topStartIdx = 0;// 194; // GH10160
	size_t botStartIdx = 0;// 1004;// GH10163
	size_t topFinishIdx = topData.videoSrc->frameCount();
	size_t botFinishIdx = botData.videoSrc->frameCount();
	size_t topFramesToRender = topFinishIdx - topStartIdx;
	size_t botFramesToRender = botFinishIdx - botStartIdx;
	size_t netFramesToRender = std::max(topFramesToRender,botFramesToRender);
	topData.seeker->seekToIdx(topStartIdx);
	botData.seeker->seekToIdx(botStartIdx);
	for (size_t ff=0; ! stop_app && ff<netFramesToRender; ff++)
	{
		rFrame.setTo(cv::Scalar(0,0,0));// clear frame
		topData.seeker->next();
		botData.seeker->next();
		uint64_t frameStart_usec = getTicks_usec();
		if (prevFrameStart_usec != 0)
		{
			int64_t meas_frameTime_usec = frameStart_usec - prevFrameStart_usec;
			frameTimeErr_usec = meas_frameTime_usec - frameTime_usec;
			// printf("frameTimeErr_usec = %ld\n",frameTimeErr_usec);
		}

		// show render progress
		if ( ! opts.showPreview)
		{
			bar.progress(ff,netFramesToRender);
		}

		try
		{
			topVideoObject.render(
				rFrame,
				RENDERED_VIDEO_SIZE.width-topVideoSize.width,
				0,
				topVideoSize);
		}
		catch (const std::runtime_error &re)
		{
			printf("caught std::runtime_error on topVideoObject.render().\n what(): %s\n",re.what());
			continue;
		}
		try
		{
			botVideoObject.render(
				rFrame,
				RENDERED_VIDEO_SIZE.width-botVideoSize.width,
				RENDERED_VIDEO_SIZE.height-botVideoSize.height,
				botVideoSize);
		}
		catch (const std::runtime_error &re)
		{
			printf("caught std::runtime_error on botVideoObject.render().\n what(): %s\n",re.what());
			continue;
		}

		topSpeedoObject.render(
			rFrame,
			rFrame.cols - topVideoSize.width - topSpeedoRenderSize.width,
			topVideoSize.height - topSpeedoRenderSize.height,
			topSpeedoRenderSize);
		botSpeedoObject.render(
			rFrame,
			rFrame.cols - botVideoSize.width - botSpeedoRenderSize.width,
			RENDERED_VIDEO_SIZE.height - botSpeedoRenderSize.height,
			botSpeedoRenderSize);

		topFC.render(
			rFrame,
			rFrame.cols - topVideoSize.width - topFC_RenderSize.width,
			0,
			topFC_RenderSize);
		botFC.render(
			rFrame,
			rFrame.cols - botVideoSize.width - botFC_RenderSize.width,
			RENDERED_VIDEO_SIZE.height-botVideoSize.height,
			botFC_RenderSize);

		trackMap.render(rFrame,0,0,tmRenderSize);

		lapTimer.render(rFrame,rFrame.cols/2,0,ltRenderSize);

		if (topPrintoutObject.isVisible())
		{
			topPrintoutObject.render(
				rFrame,
				RENDERED_VIDEO_SIZE.width-topVideoSize.width,
				0);
		}
		if (botPrintoutObject.isVisible())
		{
			botPrintoutObject.render(
				rFrame,
				RENDERED_VIDEO_SIZE.width-botVideoSize.width,
				RENDERED_VIDEO_SIZE.height-botVideoSize.height);
		}

		// write frame to video file
		vWriter.write(rFrame);

		// Display the frame live
		if (opts.showPreview)
		{
			cv::resize(rFrame,pFrame,PREVIEW_VIDEO_SIZE);
			cv::imshow("Preview", pFrame);
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

	if ( ! opts.showPreview)
	{
		bar.finish();
	}

	vWriter.release();// close video file

	return 0;
}