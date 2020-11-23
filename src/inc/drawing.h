#ifndef __DRAWING_H__
#define __DRAWING_H__

#ifndef WINDOW_X
#define WINDOW_X 400
#endif
#ifndef WINDOW_Y
#define WINDOW_Y 400
#endif

typedef void *(*setup_f_t)(void *);
typedef void (*render_f_t)(void *);
typedef void (*scale_f_t)(int, int, void *);

int drawing(setup_f_t, render_f_t, scale_f_t, void *, int);

#endif
