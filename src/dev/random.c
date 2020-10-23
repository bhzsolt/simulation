#include <random.h>
#include <unistd.h>

gsl_rng *rng;

void init_rng()
{
	rng = gsl_rng_alloc(gsl_rng_mt19937);
	gsl_rng_set(rng, getpid());
}

void cleanup_rng()
{
	gsl_rng_free(rng);
}

double rand_u(void)
{
	return gsl_rng_uniform(rng);
}

double rand_snd(void)
{
	return gsl_ran_gaussian_ziggurat(rng, 1);
}

double rand_uniform(double a, double b)
{
	return a + gsl_rng_uniform(rng) * (b - a);
}

double rand_normal(double mu, double sigma)
{
	return mu + gsl_ran_gaussian_ziggurat(rng, sigma);
}
