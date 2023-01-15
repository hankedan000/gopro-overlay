#include <csignal>
#include <getopt.h>
#include <iostream>
#include <unistd.h>// sleep
#include <time.h>

#include "GoProOverlay/data/DataSource.h"
#include "GoProOverlay/graphics/FrictionCircleObject.h"
#include "GoProOverlay/graphics/LapTimerObject.h"
#include "GoProOverlay/graphics/RenderEngine.h"
#include "GoProOverlay/graphics/SpeedometerObject.h"
#include "GoProOverlay/graphics/TelemetryPrintoutObject.h"
#include "GoProOverlay/graphics/TrackMapObject.h"
#include "GoProOverlay/graphics/VideoObject.h"
#include "tqdm.h"

const char *PROG_NAME = "single_overlay";
bool stop_app = false;

struct ProgOptions
{
	std::string inputFile;
	std::string outputFile = "single_render.mp4";
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
	printf("usage: %s -i <video_file> [options]\n",PROG_NAME);
	printf(" -i,--inputFile      : the input video file\n");
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
		{"inputFile"          , required_argument , 0                      , 'i' },
		{"outputFile"         , required_argument , 0                      , 'o' },
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
			"hi:o:",
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
			case 'i':
				opts.inputFile = optarg;
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

	if (opts.inputFile.empty())
	{
		printf("no input video file provided.\n");
		displayUsage();
		exit(0);
	}

	printf("opening %s\n", opts.inputFile.c_str());
	auto data = gpo::DataSource::loadDataFromVideo(opts.inputFile);
	if ( ! data)
	{
		printf("No video data\n");
		return -1;
	}

	auto track = data->makeTrack();
	data->setDatumTrack(track);

	auto engine = gpo::RenderEngineFactory::singleVideo(data);

	// disable LapTimerObjects (useless without a meaningful track datum)
	for (size_t ee=0; ee<engine->entityCount(); ee++)
	{
		auto &entity = engine->getEntity(ee);
		if (entity.rObj->typeName() == "LapTimerObject")
		{
			entity.rObj->setVisible(false);
		}
	}

	const auto RENDERED_VIDEO_SIZE = data->videoSrc->frameSize();
	const auto PREVIEW_VIDEO_SIZE = cv::Size(1280,720);
	double frameCount = data->videoSrc->frameCount();
	double fps = data->videoSrc->fps();
	double frameTime_sec = 1.0 / fps;
	double frameTime_usec = 1.0e6 / fps;
	int frameTime_ms = std::round(1.0e3 / fps);

	cv::Mat pFrame;// preview frame
	gpo::VideoObject videoObject;

	cv::VideoWriter vWriter(
		opts.outputFile,
		cv::VideoWriter::fourcc('M','P','4','V'),
		fps,
		RENDERED_VIDEO_SIZE,
		true);
	tqdm bar;// for render progress
	uint64_t prevFrameStart_usec = 0;
	int64_t frameTimeErr_usec = 0;// (+) means measured frame time was longer than targeted FPS
	size_t initFrameIdx = 0;
	size_t netFramesToRender = frameCount - initFrameIdx;
	data->seeker->seekToIdx(initFrameIdx);
	for (size_t ff=0; ! stop_app && ff<netFramesToRender; ff++)
	{
		data->seeker->next();
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

		engine->render();

		// write frame to video file
		vWriter.write(engine->getFrame());

		// Display the frame live
		if (opts.showPreview)
		{
			cv::resize(engine->getFrame(),pFrame,PREVIEW_VIDEO_SIZE);
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

		prevFrameStart_usec = frameStart_usec;
	}

	if ( ! opts.showPreview)
	{
		bar.finish();
	}

	vWriter.release();// close video file

	return 0;
}