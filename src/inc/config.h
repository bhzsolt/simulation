#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <inttypes.h>

struct config {
	struct box {
		double SX;					/* 60.0 * sqrt(1.25) */
		double SY;					/* 60.0 * sqrt(1.25) */
		double sx;					/* SX / 2 */
		double sy;					/* SY / 2 */
	} box;
	double particle_driving_force;	/* f = 0.5 */
	double r_0; 					/* r_0 = 4.0 */
	double r_0_2;					/* 16.0 */
	double r_min;					/* 0.2 */
	double dt;						/* 0.001 */
	double temperature;				/* 0.0 */
	uint_fast16_t n;				/* 1000 */
};

struct time {
	uint_fast16_t total;			/* 100000 */
	uint_fast16_t echo;				/* 1000 */
	uint_fast16_t movie;			/* 100 */
};

struct arguments {
	struct config config;
	struct time time;
	const char *movie_filename;		/* "particles.mvi" */
	const char *timer_filename;		/* "elapsed.times */
	double specific_volume;			/* 4.5 */
	uint_fast8_t no_messages;		/* 0 */
	uint_fast8_t help;				/* 0 */
	uint_fast8_t error;				/* 0 */
};

struct arguments parse_args(int, char * const []);
void check_system_density(struct arguments *);
void check_filenames(struct arguments *);

#endif /* config.h */
