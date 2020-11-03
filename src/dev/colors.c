#include <stdlib.h>
#include <string.h>
#include <colors.h>

float *create_colors(int n)
{
	float *colors = calloc(3 * n, sizeof(*colors));
	static const float tmp[] = COLOR_CODES;
	colors = memcpy(colors, tmp, 3 * n * sizeof(*colors));
	return colors;
}
