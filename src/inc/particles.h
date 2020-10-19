#ifndef PARTICLES
#define PARTICLES

struct coord {
	double x;
	double y;
};

struct force {
	double x;
	double y;
};

struct particles {
	struct coord coord;
	struct force force;
	int color;
	double direction;
};

#endif /* particles.h */
