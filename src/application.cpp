#include <csignal>
#include <getopt.h>
#include <iostream>
#include <unistd.h>// sleep

const char *PROG_NAME = "application";

static bool stop_app = false;

struct prog_options
{
	std::string cfg_filepath = "default.cfg";
	int daemon_flag = 0;
	int debug_flag = 0;
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
	printf("usage: %s [options]\n",PROG_NAME);
	printf(" --cfg-file       : path to a config file\n");
	printf(" --daemon         : run as daemon process\n");
	printf(" --debug          : enable debugging\n");
	printf(" -h,--help        : display this menu\n");
}

void
parse_args(
	int argc,
	char *argv[],
	prog_options &opts)
{
	static struct option long_options[] =
	{
		{"cfg-file"      , required_argument, 0                   , 'c'},
		{"daemon"        , no_argument      , &opts.daemon_flag  , 1  },
		{"debug"         , no_argument      , &opts.debug_flag   , 1  },
		{"help"          , no_argument      , 0                   , 'h'},
		{0, 0, 0, 0}
	};

	while (true)
	{
		int option_index = 0;
		int c = getopt_long(
			argc,
			argv,
			"c:h",
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
			case 'c':
				opts.cfg_filepath = optarg;
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

int main(int argc, char *argv[])
{
	signal(SIGINT, handle_sigint);// ctrl+c to stop application

	prog_options opts;
	parse_args(argc,argv,opts);

	printf("cfg_filepath = %s\n", opts.cfg_filepath.c_str());
	printf("daemon_flag  = %d\n", opts.daemon_flag);
	printf("debug_flag   = %d\n", opts.debug_flag);

	while ( ! stop_app)
	{
		printf("hello!\n");
		sleep(1);
	}

	printf("goodbye!\n");

	return 0;
}