#ifndef PARTICLE
#define PARTICLE

#define RED		1
#define BLUE	2
#define GREEN	3
#define GREY0	4
#define GREY1	5

#ifndef PARTICLE_COLORS_ONLY
#include <inttypes.h>
#include <config.h>

struct particle {
	struct coord {
		double x;
		double y;
	} coord;
	struct force {
		double x;
		double y;
	} force;
	double direction; //
	uint_fast8_t color;
};

int particles_positioning(struct particle *, struct config *);
void particles_calculate_external_force(struct particle *, struct config *);
//void particles_calculate_thermal(struct particle *, struct config *);
int particles_calculate_pairwise_forces(struct particle *, struct config *);
void particles_move(struct particle *, struct config *);
void particles_print_to_file(struct particle *, struct config *, uint_fast16_t, FILE *);

#endif /* particle structure */
#endif /* particle.h */

