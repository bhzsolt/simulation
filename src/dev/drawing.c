#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <model.h>
#include <colors.h>

struct drawing_data {
	//int n = color_n + coord_n;
	//int size = sizeof(type) * 3[x,y,c] * npoints;
	struct attrib_pointer_data {
		size_t type_size;
		int coord_n;
		int color_n;
	} attrib_pointer_data;
	void **particle_data_pointer;
	int wx;
	int wy;
	int fullscreen;
	float radius;
	int n_segments;
	int n_points;
};

inline size_t apd_sub_size(struct drawing_data *data)
{
	return (data->attrib_pointer_data.coord_n + data->attrib_pointer_data.color_n) 
		* data->n_points;
}

inline size_t apd_size(struct drawing_data *data)
{
	return data->attrib_pointer_data.type_size * apd_sub_size(data);
}

struct models {
	struct model *particles;
	struct model *frame;
};

GLFWwindow *create_window(int, int, char);

void error_callback(int, const char *);
void key_pressed_callback(GLFWwindow *, int, int, int, int);
void framebuffer_size_callback(GLFWwindow *, int, int);
static void set_callbacks(GLFWwindow *);

static void glfwMainLoop(GLFWwindow *, void (*)(void *), void *);

static GLfloat *create_rotation_matrix(int);
static GLfloat *create_scale_matrix(float, int, int);

void GLAPIENTRY gl_error_callback(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar *, const void *);

void render(void *);

int drawing(struct drawing_data *, struct model_specs *, struct model_specs *);

int main(int argc, char * const argv[])
{
	static GLfloat tmp[] = {
		-0.5f, -0.5f, BLUE,
		0.5f, -0.5f, RED,
		0.5f, 0.5f, GREEN,
		-0.5f, 0.5f, GREY0,
		0.f, 0.f, GREY1
	};
	static GLfloat tmp2[] = { 0.0f, 0.0f };
	GLfloat *frame_vertices = calloc(2, sizeof *frame_vertices);
	frame_vertices = memcpy(frame_vertices, tmp2, sizeof *frame_vertices * 2);
	if (!frame_vertices) {
		perror("initializing frame vertices\n");
		exit(EXIT_FAILURE);
	}
	static struct drawing_data data = {
		.attrib_pointer_data = {
			.type_size = sizeof *tmp,
			.coord_n = 2,
			.color_n = 1
		},
		.wx = 600,
		.wy = 600,
		.radius = 0.1,
		.n_segments = 36,
		.n_points = 5
	};
	GLfloat *particle_vertices = calloc(apd_sub_size(&data), sizeof *particle_vertices);
	particle_vertices = memcpy(particle_vertices, tmp, apd_size(&data));
	if (!particle_vertices) {
		perror("initializing particle vertices\n");
		exit(EXIT_FAILURE);
	}
	data.fullscreen = atoi(argv[1]);

	for (int i = 0; i < data.n_points; ++i) {
		printf("[%.2f, %.2f] -> %.0f\n", particle_vertices[3*i],
				particle_vertices[3 * i + 1],
				particle_vertices[3 * i + 2]);
	}

	GLfloat *rmat = create_rotation_matrix(data.n_segments);
	GLfloat *smat = create_scale_matrix(0.8, data.wx, data.wy);
	float *colors = create_colors(5);

	char * const particle_shaders[] = {"particle.vert", "particle.frag", "particle.geom"};
	struct model_specs particles = {
		.name	= "particles",
		.data	= particle_vertices,
		.size	= sizeof(GLfloat) * 15,
		.n_attribute = 2,
		.n_uniform	= 5,
		.shader_source = particle_shaders,
		.n_shader	= 3,
		.n_points	= 5,
		.usage		= GL_STREAM_DRAW,
		.mode		= GL_POINTS
	};
	struct attribute *particle_attributes = calloc(particles.n_attribute, sizeof *particle_attributes);
	struct uniform_specs *particle_uniforms = calloc(particles.n_uniform, sizeof *particle_uniforms);

	add_attribute("position", 
				  2,  
				  GL_FLOAT, 
				  sizeof(GLfloat) * 3, 
				  (const void *) 0,
				  particle_attributes);
	add_attribute("color", 
				  1,
				  GL_FLOAT, 
				  sizeof(GLfloat) * 3,
				  (const void *)(2 * sizeof(GLfloat)),
				  particle_attributes + 1);
	
	add_float_uniform("radius", &(data.radius), particle_uniforms);
	add_int_uniform("n_segments", &(data.n_segments), particle_uniforms + 1);
	add_matrix_uniform("rotation_matrix", rmat, particle_uniforms + 2);
	add_matrix_uniform("scale_matrix", smat, particle_uniforms + 3);
	add_vect3_uniform("colors", colors, 5, particle_uniforms + 4);

	particles.attributes = particle_attributes;
	particles.uniforms = particle_uniforms;
	
	float *smat_2 = create_scale_matrix(0.9, data.wx, data.wy);
	char * const frame_shaders[] = {"frame.vert", "frame.frag", "frame.geom"};
	struct model_specs frame = {
		.name	= "frame",
		.data	= (void *)frame_vertices,
		.size	= sizeof *frame_vertices * 2,
		.n_attribute = 1,
		.n_uniform	= 1,
		.shader_source = frame_shaders,
		.n_shader	= 3,
		.n_points	= 1,
		.usage		= GL_STATIC_DRAW,
		.mode		= GL_POINTS
	};

	struct attribute frame_attribute = {
		.name		= "position",
		.pointer	= (void *) 0,
		.size		= 2,
		.type		= GL_FLOAT,
		.stride		= 0,
		.normalized	= GL_FALSE
	};
	struct uniform_specs frame_uniform = {
		.name		= "scale_matrix",
		.data		= smat_2,
		.type		= U_M4FV,
		.count		= 1,
		.transpose	= GL_TRUE
	};

	frame.attributes = &frame_attribute;
	frame.uniforms = &frame_uniform;
	
	int retval = drawing(&data, &particles, &frame);
	//printf("returned");

	free(smat_2);
	free(colors);
	free(smat);
	free(rmat);
	free(particle_vertices);
	free(frame_vertices);
	exit(retval);

}

int drawing(struct drawing_data *data, struct model_specs *particles_s, struct model_specs *frame_s)
{
	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) return EXIT_FAILURE;

	GLFWwindow *window;
	if (!(window = create_window(data->wx, data->wy, data->fullscreen))) {
		glfwTerminate();
		return EXIT_FAILURE;
	}

	set_callbacks(window);

	glewExperimental = GL_TRUE;
	glewInit();
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(gl_error_callback, 0);
	
	struct model *particles = create_model(particles_s);
	struct model *frame;// = create_model(frame_s);

	glClearColor(1.f, 1.f, 1.f, 0.f);
	//glLineWidth(1.5);
	
	struct models models = {
		.particles = particles,
		.frame = frame
	};

	glfwSetWindowUserPointer(window, &models);

	glfwMainLoop(window, render, &models);

	cleanup_model(particles);
	//cleanup_model(frame);
	glfwDestroyWindow(window);
	glfwTerminate();
	return EXIT_SUCCESS;
}


GLFWwindow *create_window(int width, int height, char fullscreen)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	
	GLFWwindow *window;
	if (fullscreen == 0 || fullscreen == 'n' || fullscreen == 'N') {
		if (!(window = glfwCreateWindow(width, height, "Brownian Dynamics Simulation Plotter", NULL, NULL))) return NULL;
		glfwSetWindowPos(window, (1920 - width) / 2, (1080 - height) / 2);
	} else {
		if (!(window = glfwCreateWindow(width, height, "Brownian Dynamics Simulation Plotter", glfwGetPrimaryMonitor(), NULL))) return NULL;
	}
	glfwMakeContextCurrent(window);
	glfwSetWindowSizeLimits(window, 200, 200, 1000, 1000);
	glfwSetWindowAspectRatio(window, 1, 1);
	glfwSwapInterval(1);
	return window;
}


void error_callback(int error, const char *description)
{
	fprintf(stderr, "[ERROR]: %s\n", description);
	exit(EXIT_FAILURE);
}

void key_pressed_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);

	struct models *models = glfwGetWindowUserPointer(window);
	//GLfloat *smat_f = create_scale_matrix(0.9, width, height);
	GLfloat *smat_p = create_scale_matrix(0.8, width, height);

	//set_uniform("scale_matrix", smat_f, models->frame);
	set_uniform("scale_matrix", smat_p, models->particles);

	free(smat_p);
	//free(smat_f);
}

static void set_callbacks(GLFWwindow *window)
{
	glfwSetKeyCallback(window, key_pressed_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
}


static void glfwMainLoop(GLFWwindow *window, void (*render_function)(void *arguments), void *arguments)
{
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);
		render_function(arguments);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

static GLfloat *create_rotation_matrix(int n)
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

static GLfloat *create_scale_matrix(float factor, int width, int height)
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


void render(void *arguments)
{
	struct models *models = (struct models *)arguments;
	draw_model(models->particles);
	//draw_model(models->frame);
}


void GLAPIENTRY gl_error_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	fprintf(stderr, "[GL]: %s type = 0x%x, severity = 0x%x, id = 0x%x%s%s\n",
			(type == GL_DEBUG_TYPE_ERROR ? "[ERROR]: " : ""), type, severity, id, (length != 0 ? ", message = " : ""), message);
}

