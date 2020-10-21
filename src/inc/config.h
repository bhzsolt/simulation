#ifndef SETUP
#define SETUP

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
	uint_fast8_t help;
};

struct arguments parse_args(int, char * const []);

#endif /* setup.h */
