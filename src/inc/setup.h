#ifndef SETUP
#define SETUP

struct box {
	double SX;
	double SY;
	double sx;
	double sy;
};

struct config {
	int n;
	double particle_driving_force; /* f */
	double screening_length; /* r_0 */
	double dt;
};

struct time {
	int total;
	int echo;
	int movie;
};

#endif /* setup.h */
