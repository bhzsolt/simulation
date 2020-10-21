#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <config.h>

static struct option long_options[] = {
	{"box-x",		required_argument,	NULL,	'x'},
	{"box-y",		required_argument,	NULL, 	'y'},
	{"force",		required_argument,	NULL,	'f'},
	{"r_0",			required_argument,	NULL,	'R'},
	{"r_min",		required_argument,	NULL,	'r'},
	{"time-step",	required_argument,	NULL,	't'},
	{"n-particles",	required_argument,	NULL,	'n'},
	{"total-time",	required_argument,	NULL,	'T'},
	{"echo-time",	required_argument,	NULL,	'e'},
	{"movie-time",	required_argument,	NULL,	'm'},
	{"help",		no_argument,		NULL,	'h'},
	{NULL,			0,					NULL,	0}
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
			.n		= 800
		},
		.time = {
			.total	= 100000,
			.echo	= 1000,
			.movie	= 100
		},
		.help = 0
	};

	int opt;
	while ((opt = getopt_long(argc, argv, "x:y:f:R:r:t:n:T:e:m:h", long_options, NULL)) != -1) {
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
		case 'h':
			arguments.help = 1;
			break;
		}		
	}

	return arguments;
}
