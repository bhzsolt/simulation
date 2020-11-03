#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <shaders.h>
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
	return (data->attrib_pointer_data.coord_n + data->attrib_pointer_data.color_n) * data->n_points;
}

inline size_t apd_size(struct drawing_data *data)
{
	return data->attrib_pointer_data.type_size * apd_sub_size(data);
}

struct render_args {
	int n_points;
	int index;
};

GLFWwindow *create_window(int, int, char);
void error_callback(int, const char *);
void key_pressed_callback(GLFWwindow *, int, int, int, int);
void framebuffer_size_callback(GLFWwindow *, int, int);
static void set_callbacks(GLFWwindow *);
static void glfwMainLoop(GLFWwindow *, void (*)(void *), void *);
int create_opengl_objects(GLuint *, GLuint *, int, void *);
int handling_uniforms(GLuint, float, int, int, int);
int handling_attribute_pointers(GLuint, struct attrib_pointer_data *);

static GLfloat *create_rotation_matrix(int);
static GLfloat *create_scale_matrix(int, int);

void render(void *);

int drawing(struct drawing_data *, int, char * const []);

int main(int argc, char * const argv[])
{
	static GLfloat tmp[] = {
		-0.5f, -0.5f, 1.f,
		0.5f, -0.5f, 2.f,
		0.5f, 0.5f, 3.f,
		-0.5f, 0.5f, 4.f,
		0.f, 0.f, 5.f
	};
	static struct drawing_data data = {
		.attrib_pointer_data = {
			.type_size = sizeof(*tmp),
			.coord_n = 2,
			.color_n = 1
		},
		.wx = 600,
		.wy = 600,
		.radius = 0.1,
		.n_segments = 36,
		.n_points = 5
	};
	GLfloat *circle_vertices = calloc(data.attrib_pointer_data.type_size, apd_sub_size(&data));
	circle_vertices = memcpy(circle_vertices, tmp, apd_size(&data));
	if (!circle_vertices) {
		perror("initializing vertices\n");
		exit(EXIT_FAILURE);
	}
	data.particle_data_pointer = (void **)&circle_vertices;
	data.fullscreen = atoi(argv[1]);
	int retval = drawing(&data, argc - 2, argv + 2);
	free(circle_vertices);
	exit(retval);
}

int drawing(struct drawing_data *data, int n_shader, char * const shader_paths[])
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

	GLuint vao, vbo; 
	if (create_opengl_objects(&vao, &vbo, apd_size(data), data->particle_data_pointer[0])) return EXIT_FAILURE;

	GLuint program = create_shader_program(n_shader, shader_paths);
	if (!program) {
		perror("create shader program");
		return EXIT_FAILURE;
	}
	glUseProgram(program);

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glfwSetWindowUserPointer(window, &program);

	if (handling_attribute_pointers(program, &(data->attrib_pointer_data))) return EXIT_FAILURE;
	if (handling_uniforms(program, data->radius, data->n_segments, width, height)) return EXIT_FAILURE;

	glClearColor(1.f, 1.f, 1.f, 0.f);

	struct render_args render_args = {
		.n_points = data->n_points,
		.index = 0
	};
	glfwMainLoop(window, render, &render_args);

	glDeleteProgram(program);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);

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

	if (width != height) {
		GLuint *program = glfwGetWindowUserPointer(window);
		GLint uni_scale = glGetUniformLocation(*program, "scale_matrix");
		if (GL_NO_ERROR != glGetError()) {
			fprintf(stderr, "[ERROR]: framebuffer size callback: getting scale matrix uniform\n");
		}

		GLfloat *scale = create_scale_matrix(width, height);
		if (!scale) {
			perror("framebuffer size callback: create scale matrix");
		}
		glUniformMatrix4fv(uni_scale, 1, GL_TRUE, scale);
		if (GL_NO_ERROR != glGetError()) {
			fprintf(stderr, "[ERROR]: framebuffer size callback: setting scale matrix uniform\n");
		}
		free(scale);
	}
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

int create_opengl_objects(GLuint *vao, GLuint *vbo, int size, void *pointer)
{
	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);
	if (GL_NO_ERROR != glGetError()) {
		fprintf(stderr, "[ERROR]: binding vertex array object\n");
		return 1;
	}

	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	//GL_DYNAMIC_DRAW
	glBufferData(GL_ARRAY_BUFFER, size, pointer, GL_STREAM_DRAW);
	if (GL_NO_ERROR != glGetError()) {
		fprintf(stderr, "[ERROR]: binding vertex buffer object\n");
		return 1;
	}
	return 0;
}

int handling_uniforms(GLuint program, float radius, int n_segments, int width, int height)
{
#ifdef CHECK_ERRORS
#	error "handling_uniforms: CHECK_ERRORS previously defined"
#elif defined INIT || defined ERROR || defined RMAT || defined COLORS || defined SCALE
#	error "flags already defined"
#endif
#define CHECK_ERRORS(error, string) do { if (GL_NO_ERROR != glGetError()) { fprintf(stderr, "[ERROR]: %s\n", (string)); flags |= error; goto cleanup; } } while (0)
#define INIT	0x0
#define ERROR	0x1
#define RMAT 	0x2
#define COLORS	0x4
#define SCALE	0x8
	int flags = INIT;
	
	GLint uni_r = glGetUniformLocation(program, "radius");
	CHECK_ERRORS(ERROR, "getting radius uniform");
	GLint uni_n = glGetUniformLocation(program, "n_segments");
	CHECK_ERRORS(ERROR, "getting n segments uniform");
	GLint uni_r_mat = glGetUniformLocation(program, "rotation_matrix");
	CHECK_ERRORS(ERROR, "getting rotation matrix uniform");
	GLint uni_scale = glGetUniformLocation(program, "scale_matrix");
	CHECK_ERRORS(ERROR, "getting scale matrix uniform");
	GLint uni_colors = glGetUniformLocation(program, "colors");
	CHECK_ERRORS(ERROR, "getting colors uniform");

	glUniform1f(uni_r, radius);
	CHECK_ERRORS(ERROR, "setting radius uniform");
	glUniform1i(uni_n, n_segments);
	CHECK_ERRORS(ERROR, "setting n segments uniform");
	
	GLfloat *rmat = create_rotation_matrix(n_segments);
	if (!rmat) {
		perror("create rotation matrix");
		flags |= ERROR;
		goto cleanup;
	}
	flags |= RMAT;
	glUniformMatrix4fv(uni_r_mat, 1, GL_TRUE, rmat);
	CHECK_ERRORS(ERROR, "setting rotation matrix uniform");

	GLfloat *scale = create_scale_matrix(width, height);
	if (!scale) {
		perror("create scale matrix");
		flags |= ERROR;
		goto cleanup;
	}
	flags |= SCALE;
	glUniformMatrix4fv(uni_scale, 1, GL_TRUE, scale);
	CHECK_ERRORS(ERROR, "setting scale matrix uniform");

	GLfloat *color_values = create_colors(5);
	if (!color_values) {
		perror("create colors");
		flags |= ERROR;
		goto cleanup;
	}
	flags |= COLORS;
	glUniform3fv(uni_colors, 5, color_values);
	CHECK_ERRORS(ERROR, "setting colors uniform");

cleanup:
	if (flags & COLORS) free(color_values);
	if (flags & SCALE) free(scale);
	if (flags & RMAT) free(rmat);
	flags &= ERROR;
	return flags;
#undef INIT
#undef ERROR
#undef RMAT
#undef COLORS
#undef SCALE
#undef CHECK_ERRORS
}

int handling_attribute_pointers(GLuint program, struct attrib_pointer_data *data)
{
#ifdef CHECK_ATTRIBUTE
#	error "CHECK ATTRIBUTE already defined"
#elif defined CHECK_ERRORS
#	error "CHECK_ERRORS already defined"
#endif
#define STR(x) #x
#define CHECK_ATTRIBUTE(attribute) do {if ((attribute) == -1) {fprintf(stderr, "[ERROR]: %s: no such attribute\n", STR(attribute)); error = 1; goto exit;}} while (0)
#define CHECK_ERRORS(string) do {if (GL_NO_ERROR != glGetError()) {fprintf(stderr, "[ERROR]: %s\n", (string)); error = 1; goto exit;}} while (0)
	int n = data->color_n + data->coord_n;
	int error = 0;

	GLint position_attribute = glGetAttribLocation(program, "position"); 
	CHECK_ATTRIBUTE(position_attribute);
	CHECK_ERRORS("getting position attribute");
	
	GLint color_attribute = glGetAttribLocation(program, "color");
	CHECK_ATTRIBUTE(color_attribute);
	CHECK_ERRORS("getting color attribute");
	
	glVertexAttribPointer(position_attribute, data->coord_n, GL_FLOAT, GL_FALSE, n * data->type_size, (GLvoid *) 0);
	CHECK_ERRORS("setting position attribute pointer");
	glVertexAttribPointer(color_attribute, data->color_n, GL_FLOAT, GL_FALSE, n * data->type_size, (GLvoid *)(data->coord_n * data->type_size)); 
	CHECK_ERRORS("setting color attribute pointer");
	
	glEnableVertexAttribArray(position_attribute);
	CHECK_ERRORS("enabling position attribute pointer");
	glEnableVertexAttribArray(color_attribute);
	CHECK_ERRORS("enabling color attribute pointer");
exit:
	return error;
#undef STR
#undef CHECK_ATTRIBUTE
#undef CHECK_ERRORS
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

static GLfloat *create_scale_matrix(int width, int height)
{
	register GLfloat *scale = calloc(16, sizeof(*scale));
	if (!scale) return scale;

	register int k;
	for (k = 0; k < 4; ++k) scale[4 * k + k] = 1.f;

	if (height < width) {
		scale[0] = (GLfloat) height / (GLfloat) width;
	} else if (width < height) {
		scale[5] = (GLfloat) width / (GLfloat) height;
	}

	return scale;
}

void render(void *arguments)
{
	struct render_args *args = (struct render_args *)arguments;
	glDrawArrays(GL_POINTS, 0, args->n_points);
}
