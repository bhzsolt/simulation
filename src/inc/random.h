#ifndef __RANDOM_H__
#define __RANDOM_H__

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

extern gsl_rng *rng;

void init_rng();
void cleanup_rng();

double rand_u(void);
double rand_snd(void);

double rand_uniform(double, double);
double rand_normal(double, double);

#endif /* random.h */
