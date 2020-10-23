#include <stdio.h>
#include <stdlib.h>

#include <config.h>
#include <particle.h>
#include <timing.h>
#include <random.h>

typedef enum {INIT, ERROR, RALL = 2, PALL = 4, MFOP = 8, TFOP = 16, TIOP = 32} cleanup_flags;

static void print_usage(const char *);
static void print_info(struct arguments *);
static void print_status(double);
static int initialize_files(struct arguments *, FILE **, FILE **);
static void run(struct arguments *, struct particle *, FILE *);

int main(int argc, char * const argv[])
{
	struct arguments arguments = parse_args(argc, argv);
	int flags = INIT;
	int retval = EXIT_SUCCESS;

	if (arguments.help) {
		print_usage(argv[0]);
		if (!arguments.error) exit(EXIT_SUCCESS);
		exit(EXIT_FAILURE);
	}

	check_system_density(&arguments);
	
	init_rng();
	flags |= RALL;

	struct particle *particles = calloc(arguments.config.n, sizeof(*particles));
	if (particles == NULL) {
		perror("Allocating particles");
		retval = EXIT_FAILURE;
		goto cleanup;
	}
	flags |= PALL;
	
	if (particles_positioning(particles, &(arguments.config))) {
		fprintf(stderr, "Can't place particles, system too dense!\nquitting\n");
		retval = EXIT_FAILURE;
		goto cleanup;
	}
	print_info(&arguments);

	FILE *movie_file, *timer_file;
	flags |= initialize_files(&arguments, &movie_file, &timer_file);
	if (flags & ERROR) {
		perror("");
		retval = EXIT_FAILURE;
		goto cleanup;
	}

	struct timer *timer;
	if (create_timer(&timer) == NULL) {
		perror("Creating timer");
		retval = EXIT_FAILURE;
		goto cleanup;
	}
	flags |= TIOP;

	if (start_timer(timer)) {
		perror("Starting timer");
		retval = EXIT_FAILURE;
		goto cleanup;
	}
	run(&arguments, particles, movie_file);
	if (stop_timer(timer)) {
		perror("Stopping timer");
		retval = EXIT_FAILURE;
		goto cleanup;
	}
	printf("\n");
	const char *finished_on = get_finishing_time();
	if (!arguments.no_messages) {
		printf("Run finished on: %s\n", finished_on);
		printf("Program running time was: %.2lf seconds\n", get_elapsed_time(timer));
	}
	fprintf(timer_file, "%"PRIuFAST16", %.2lf, %s\n", arguments.config.n, get_elapsed_time(timer), finished_on);

cleanup:
	if (flags & TIOP) free(timer);
	if (flags & TFOP) fclose(timer_file);
	if (flags & MFOP) fclose(movie_file);
	if (flags & PALL) free(particles);
	if (flags & RALL) cleanup_rng();
	exit(retval);
}

static void print_usage(const char *filename)
{
	printf("Pedestrian crossing molecular dynamics simulation\n");
	printf("usage: %s\n", filename);
	printf("\t-h, --help\t\tdisplay this help\n");
	printf("\t-s, --surpress-messages\trun the simulation without printing out information\n");
	printf("\t-x, --box-x=DOUBLE\tbox x dimension (def: 60.00)\n");
	printf("\t-y, --box-y=DOUBLE\tbox y dimension (def: 60.00)\n");
	printf("\t-f, --force=DOUBLE\tparticle driving force (def: 0.50)\n");
	printf("\t-R, --r_0=DOUBLE\tparticle-particle screening length (def: 4.00)\n");
	printf("\t-r, --r_min=DOUBLE\tlower limit for the distance between particles (def: 0.20)\n");
	printf("\t-t, --time-step=DOUBLE\ttime increment (def: 0.001)\n");
	printf("\t-c, --temperature=DOUBLE\ttemperature (def: 0.0)\n");
	printf("\t-n, --n-particles=UINT16\tnumber of particles in the system (def: 800)\n");
	printf("\t-T, --total-time=UINT16\ttotal run time of the simulation (def: 100 000)\n");
	printf("\t-e, --echo-time=UINT16\tinterval for printing to the screen (def: 1000)\n");
	printf("\t-m, --movie-time=UINT16\tinterval for printing to movie file (def: 100)\n");
	printf("\t-M, --movie-filename=STRING\tfilename for the movie file (def: particles.mvi)\n"); 
	printf("\t-E, --timer-filename=STRING\tfilename for the running time measurements (def: elapsed.times)\n");
	printf("\t-v, --specific-volume=DOUBLE\tspecific volume of the system (box area / number of particles) (def: 4.5)\n");
}

static void print_info(struct arguments *arguments)
{
	if (!arguments->no_messages) {
		printf("Simulation methods Lab 1, 2020\n");
		printf("Not optimized Brownian Dynamics\n");
		printf("Timestep (dt) = %lf\n", arguments->config.dt);
		printf("Screening length = %lf\n", arguments->config.r_0);
		printf("Driving force on particles = %lf\n", arguments->config.particle_driving_force);
		printf("Temperature = %lf\n", arguments->config.temperature);
		printf("Box dimensions: [x = %5.2lf, y = %5.2lf]\n", arguments->config.box.SX, arguments->config.box.SY);
		printf("System density: %lf\n", arguments->config.n / (arguments->config.box.SX * arguments->config.box.SY));
		printf("%"PRIuFAST16" particles initialized\n", arguments->config.n);
	}
}

static void print_status(double percent)
{
	uint_fast8_t i, current = (uint_fast8_t) (percent / 2.5);

	printf("\r");
	printf("[");
	for (i = 0; i < 40; ++i) {
		if (i <= current) {
			printf("=");
		} else {
			printf(" ");
		}
	}
	printf("] %.2lf", percent);
	fflush(NULL);
}

static int initialize_files(struct arguments *arguments, FILE **movie_file, FILE **timer_file)
{
	int flags = INIT;

	*movie_file = fopen(arguments->movie_filename, "wb");
	if (*movie_file == NULL) {
		fprintf(stderr, "movie_file");
		flags |= ERROR;
	}
	flags |= MFOP;

	*timer_file = fopen(arguments->timer_filename, "a");
	if (*timer_file == NULL) {
		fprintf(stderr, "timer_file");
		flags |= ERROR;
	}
	flags |= TFOP;

	return flags;
}

static void run(struct arguments *arguments, struct particle *particles, FILE *movie_file)
{
	double _total = (double) 1.0 / arguments->time.total;
	for (uint_fast16_t time = 0; time < arguments->time.total; ++time) {
		particles_calculate_external_force(particles, &(arguments->config));
		particles_calculate_pairwise_forces(particles, &(arguments->config));
		//particles_calculate_thermal();
		particles_move(particles, &(arguments->config));

		if (time % arguments->time.echo == 0) {
			print_status(100.0 * (double) time * _total); 
			//printf("Timestep: %"PRIuFAST16" / %"PRIuFAST16"\n", time, arguments->time.total);
			//fflush(stdout);
		}
		if (time % arguments->time.movie == 0) 
			particles_print_to_file(particles, &(arguments->config), time, movie_file);
	}
}

