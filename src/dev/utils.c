#include <math.h>
#include <utils.h>

GLfloat *create_rotation_matrix(int n)
{
	register GLfloat theta = 2 * M_PI / n;
	register GLfloat c = cos(theta);
	register GLfloat s = sin(theta);
	register GLfloat *rmat = calloc(16, sizeof(*rmat)); 

	if (!rmat) return rmat;
	
	register int k;
	for (k = 0; k < n; ++k) {
		if ((k / 4) == (k % 4)) rmat[k] = 1.f;
	}
	rmat[0] = c; rmat[1] = -s;
	rmat[4] = s; rmat[5] = c;

	return rmat;
}

GLfloat *create_scale_matrix(float factor, int width, int height)
{
	register GLfloat *scale = calloc(16, sizeof(*scale));
	if (!scale) return scale;

	register int k;
	for (k = 0; k < 4; ++k) scale[4 * k + k] = 1.f;
	scale[0] = scale[5] = factor;

	if (height < width) {
		scale[0] = factor * ((GLfloat) height / (GLfloat) width);
	} else if (width < height) {
		scale[5] = factor *((GLfloat) width / (GLfloat) height);
	}

	return scale;
}
