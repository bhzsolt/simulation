#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include <string.h>

#include <config.h>

static struct option long_options[] = {
	{"box-x",				required_argument,	NULL,	'x'},
	{"box-y",				required_argument,	NULL, 	'y'},
	{"force",				required_argument,	NULL,	'f'},
	{"r_0",					required_argument,	NULL,	'R'},
	{"r_min",				required_argument,	NULL,	'r'},
	{"time-step",			required_argument,	NULL,	't'},
	{"temperature",			required_argument,	NULL,	'c'},
	{"n-particles",			required_argument,	NULL,	'n'},
	{"total-time",			required_argument,	NULL,	'T'},
	{"echo-time",			required_argument,	NULL,	'e'},
	{"movie-time",			required_argument,	NULL,	'm'},
	{"movie-filename",		required_argument,	NULL,	'M'},
	{"specific-volume",		required_argument,	NULL,	'v'},
	{"surpress-messages",	no_argument,		NULL,	's'},
	{"timer-filename",		required_argument,	NULL,	'E'},
	{"help",				no_argument,		NULL,	'h'},
	{NULL,					0,					NULL,	0}
};

struct arguments parse_args(int argc, char * const argv[])
{
	struct arguments arguments = {
		.config = {
			.box = {
				.SX	= 60.0,
				.SY	= 60.0,
				.sx	= 30.0,
				.sy	= 30.0
			},
			.particle_driving_force = 0.5,
			.r_0	= 4.0,
			.r_0_2	= 16.0,
			.r_min	= 0.2,
			.dt		= 0.001,
			.temperature = 0.0,
			.n		= 800
		},
		.time = {
			.total	= 100000,
			.echo	= 1000,
			.movie	= 100
		},
		.movie_filename = "particles.mvi",
		.timer_filename = "elapsed.times",
		.specific_volume = 4.5,
		.no_messages = 0,
		.help = 0,
		.error = 0
	};

	int opt;
	while ((opt = getopt_long(argc, argv, "x:y:f:R:r:t:c:n:T:e:m:M:v:sE:h", long_options, NULL)) != -1) {
		switch (opt) {
		case 'x':
			sscanf(optarg, "%lf", &(arguments.config.box.SX));
			arguments.config.box.sx = arguments.config.box.SX / 2;
			break;
		case 'y':
			sscanf(optarg, "%lf", &(arguments.config.box.SY));
			arguments.config.box.sy = arguments.config.box.SY / 2;
			break;
		case 'f':
			sscanf(optarg, "%lf", &(arguments.config.particle_driving_force));
			break;
		case 'R':
			sscanf(optarg, "%lf", &(arguments.config.r_0));
			arguments.config.r_0_2 = arguments.config.r_0 * arguments.config.r_0;
			break;
		case 'r':
			sscanf(optarg, "%lf", &(arguments.config.r_min));
			break;
		case 't':
			sscanf(optarg, "%lf", &(arguments.config.dt));
			break;
		case 'c':
			sscanf(optarg, "%lf", &(arguments.config.temperature));
			break;
		case 'n':
			sscanf(optarg, "%"SCNuFAST16, &(arguments.config.n));
			break;
		case 'T':
			sscanf(optarg, "%"SCNuFAST16, &(arguments.time.total));
			break;
		case 'e':
			sscanf(optarg, "%"SCNuFAST16, &(arguments.time.echo));
			break;
		case 'm':
			sscanf(optarg, "%"SCNuFAST16, &(arguments.time.movie));
			break;
		case 'M':
			arguments.movie_filename = optarg;
			break;
		case 'v':
			sscanf(optarg, "%lf", &(arguments.specific_volume));
			break;
		case 's':
			arguments.no_messages = 1;
			break;
		case 'E':
			arguments.timer_filename = optarg;
			break;
		case 'h':
			arguments.help = 1;
			break;
		default:
			arguments.error = 1;
			arguments.help = 1;
			break;
		}		
	}

	return arguments;
}

void check_system_density(struct arguments *arguments)
{
	double sv = (arguments->config.box.SX * arguments->config.box.SY) / (double) arguments->config.n;
	if (sv != arguments->specific_volume) {
		fprintf(stderr, "System density is incorrect!\n");
		printf("Would you like me to correct the box size? [Y/n]: ");
		int opt = fgetc(stdin);
		if (opt != 'n') {
			arguments->config.box.SX = arguments->config.box.SY = sqrt(arguments->specific_volume * arguments->config.n);
			arguments->config.box.sx = arguments->config.box.sy = arguments->config.box.SX / 2;
		}
	}
}

void check_filenames(struct arguments *arguments)
{
	if (strrchr(arguments->movie_filename, '.') == NULL) {
		fprintf(stderr, "Error: movie filetype should be .mvi\n");
		fprintf(stderr, "using default value\n");
		arguments->movie_filename = "particles.mvi";
	} else if (strcmp(".mvi", strrchr(arguments->movie_filename, '.'))) {
		fprintf(stderr, "Error: movie filetype should be .mvi\n");
		fprintf(stderr, "using default value\n");
		arguments->movie_filename = "particles.mvi";
	}

	if (strrchr(arguments->timer_filename, '.') == NULL) {
		fprintf(stderr, "Error: timer filetype should be .times\n");
		fprintf(stderr, "using default value\n");
		arguments->timer_filename = "elapsed.times";
	} else if (strcmp(".times", strrchr(arguments->timer_filename, '.'))) {
		fprintf(stderr, "Error: timer filetype should be .times\n");
		fprintf(stderr, "using default value\n");
		arguments->timer_filename = "elapsed.times";
	}
}
