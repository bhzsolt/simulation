#include <stdio.h>
#include <stdlib.h>
#include <particle.h>

int main(void) {
	struct particle particle;
	printf("Size of particle is %ld\n", sizeof(particle));
	printf("Size of coord is %ld\n", sizeof(particle.coord));
	printf("Size of force is %ld\n", sizeof(particle.force));
	printf("Size of direction is %ld\n", sizeof(particle.direction));
	printf("Size of color is %ld\n", sizeof(particle.color));
	printf("Size of const char * is %ld\n", sizeof(const char *));
	exit(EXIT_SUCCESS);
}
