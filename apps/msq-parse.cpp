#include <csignal>
#include <getopt.h>
#include <iostream>
#include <unistd.h>// sleep
#include <spdlog/spdlog.h>
#include <time.h>

#include "GoProOverlay/utils/DataProcessingUtils.h"

const char *PROG_NAME = "msq-parse";
bool stop_app = false;

struct ProgOptions
{
	std::string inputFile;
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
	printf("usage: %s -i <msl_file> [options]\n",PROG_NAME);
	printf(" -i,--inputFile      : the input video file\n");
	printf(" -h,--help           : display this menu\n");
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
		{"help"               , no_argument       , 0                      , 'h' },
		{0, 0, 0, 0}
	};

	while (true)
	{
		int option_index = 0;
		int c = getopt_long(
			argc,
			argv,
			"hi:",
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
		spdlog::error("no input file provided.");
		displayUsage();
		exit(0);
	}

	spdlog::info("opening {}", opts.inputFile);
    std::vector<gpo::ECU_TimedSample> ecuTelem;
	auto msqRes = utils::readMegaSquirtLog(opts.inputFile, ecuTelem);
    if ( ! msqRes.first)
    {
		spdlog::error("failed to parse megasquirt log file '{}'.", opts.inputFile);
        return -1;
    }

	// compute and show some stats
	size_t numSamps = ecuTelem.size();
	float logDur_sec = 0.0;
	float logRate_hz = 0.0;
	if (numSamps > 0)
	{
		logDur_sec = ecuTelem.back().t_offset;
	}
	if (numSamps >= 1 && logDur_sec > 0.0)
	{
		logRate_hz = (numSamps - 1) / logDur_sec;
	}
	spdlog::info("num samples:  {}", numSamps);
	spdlog::info("log duration: {}s", logDur_sec);
	spdlog::info("avg log rate: {:0.3f}Hz", logRate_hz);

	return 0;
}