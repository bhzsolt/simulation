#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <particle.h>
#include <colors.h>
#include <random.h>

static double distance_folded_PBC(double, double, double, double, struct config *);
static void distance_squared_folded_PBC(struct particle *, struct particle *, double *, double *, double *, struct box *);
static void particles_fold_particle_back_PBC(struct particle *, struct box *);
static int write_int(int, FILE *);
static int write_float(float, FILE *);

int particles_positioning(struct particle *particles, struct config *config) 
{
	uint_fast16_t i, j;
	double x_try = 0.0;
	double y_try = 0.0;
	double dr;
	uint_fast8_t overlap;
	unsigned int trial_counter;
	unsigned int trial_limit = config->n * config->n;

	for (i = 0; i < config->n; ++i) {
		overlap = 1;
		trial_counter = 0;

		do {
			x_try = rand_uniform(0, config->box.SX);
			y_try = rand_uniform(0, config->box.SY);
			overlap = 0;

			j = 0;
			while (!overlap && (j < i)) {
				dr = distance_folded_PBC(x_try, y_try, particles[j].coord.x, particles[j].coord.y, config);
				if (dr < config->r_min) {
					overlap = 1;
					++trial_counter;
				}
				++j;
			}
		} while (overlap && (trial_counter < trial_limit));

		if (trial_counter >= trial_limit) {
			return -1;
		}

		particles[i].coord.x = x_try;
		particles[i].coord.y = y_try;
		particles[i].force.x = 0.0;
		particles[i].force.y = 0.0;

		if (rand_u() < 0.5) {
			particles[i].direction = - 1.0;
			particles[i].color = RED;
		} else {
			particles[i].direction = + 1.0;
			particles[i].color = BLUE;
		}
	}
	return 0;
}

void particles_calculate_external_force(struct particle *particles, struct config *config)
{
	uint_fast16_t i;

	for (i = 0; i < config->n; ++i) {
		particles[i].force.x += particles[i].direction * config->particle_driving_force;
	}
}

void particles_calculate_thermal(struct particle *particles, struct config *config)
{
	uint_fast16_t i;

	for (i = 0; i < config->n; ++i) {
		particles[i].force.x += config->temperature * rand_snd();
		particles[i].force.y += config->temperature * rand_snd();
	}
}

int particles_calculate_pairwise_forces(struct particle *particles, struct config *config)
{
	uint_fast16_t i, j;
	double r, r2, f;
	double dx, dy;
	uint_fast16_t ni = config->n - 1;

	for (i = 0; i < ni; ++i) {
		for (j = i + 1; j < config->n; ++j) {
			distance_squared_folded_PBC(&particles[i], &particles[j], &r2, &dx, &dy, &(config->box));

			if (r2 < config->r_0_2) {
				if (r2 < 0.01) {
					fprintf(stderr, "Error: particles too close!\nquitting\n");
					return -1;
				} else {
					r = sqrt(r2);
					f = 1.0/r2 * exp(-r / config->r_0);
				}

				f = f / r;

				particles[i].force.x -= f * dx;
				particles[i].force.y -= f * dy;

				particles[j].force.x += f * dx;
				particles[j].force.y += f * dy;
			}
		}
	}
	return 0;
}

void particles_move(struct particle *particles, struct config *config)
{
	uint_fast16_t i;

	for (i = 0; i < config->n; ++i) {
		particles[i].coord.x += particles[i].force.x * config->dt;
		particles[i].coord.y += particles[i].force.y * config->dt;

		particles_fold_particle_back_PBC(&particles[i], &(config->box));

		particles[i].force = (struct force){.x = 0.0, .y = 0.0};
	}
}

void particles_print_to_file(struct particle *particles, struct config *config, uint_fast16_t time, FILE *file)
{
	uint_fast16_t i;

	write_int(config->n, file);
	write_int(time, file);

	for (i = 0; i < config->n; ++i) {
		write_int(particles[i].color, file);
		write_int(i, file);
		write_float(particles[i].coord.x, file);
		write_float(particles[i].coord.y, file);
		write_float(1.0, file);
	}
}

/* static methods */

static double distance_folded_PBC(double x0, double y0, double x1, double y1, struct config *config) 
{
	double dx = x1 - x0;
	double dy = y1 - y0;

	if (dx > config->box.sx) {
		dx -= config->box.SX;
	} else if (dx <= -config->box.sx) {
		dx += config->box.SX;
	}

	if (dy > config->box.sy) {
		dy -= config->box.SY;
	} else if (dy <= -config->box.sy) {
		dy += config->box.SY;
	}

	return sqrt(dx * dx + dy * dy);
}

static void distance_squared_folded_PBC(struct particle *pi, struct particle *pj, double *r2, double *dx, double *dy, struct box *box)
{
	*dx = pj->coord.x - pi->coord.x;
	*dy = pj->coord.y - pi->coord.y;

	if (*dx > box->sx) {
		*dx -= box->SX;
	} else if (*dx <= -box->sx) {
		*dx += box->SX;
	}

	if (*dy > box->sy) {
		*dy -= box->SY;
	} else if (*dy <= -box->sy) {
		*dy += box->SY;
	}

	*r2 = *dx * *dx + *dy * *dy;
}

static void particles_fold_particle_back_PBC(struct particle *particle, struct box *box)
{
	if (particle->coord.x < 0) {
		particle->coord.x += box->SX;
	} else if (particle->coord.x >= box->SX) {
		particle->coord.x -= box->SX;
	}
	if (particle->coord.y < 0) {
		particle->coord.y += box->SY;
	} else if (particle->coord.y >= box->SY) {
		particle->coord.y -= box->SY;
	}
}

static int write_int(int intholder, FILE *file)
{
	return fwrite(&intholder, sizeof(int), 1, file);
}

static int write_float(float floatholder, FILE *file)
{
	return fwrite(&floatholder, sizeof(float), 1, file);
}
