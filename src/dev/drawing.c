#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <drawing.h>

struct win_u_point_d {
	void *args;
	scale_f_t scale_function;
};

static GLFWwindow *create_window(int, int, char);
void error_callback(int, const char *);
void key_pressed_callback(GLFWwindow *, int, int, int, int);
void framebuffer_size_callback(GLFWwindow *, int, int);
static void set_callbacks(GLFWwindow *);
static void glfwMainLoop(GLFWwindow *, render_f_t, void *);
void GLAPIENTRY gl_error_callback(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar *, const void *);


int drawing(setup_f_t setup_function, render_f_t render_function, scale_f_t scale_function, void *setup_args, int fullscreen)
{
	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) return EXIT_FAILURE;

	GLFWwindow *window;
	if (!(window = create_window(WINDOW_X, WINDOW_Y, fullscreen))) {
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glewExperimental = GL_TRUE;
	glewInit();
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(gl_error_callback, 0);

	void *args = setup_function(setup_args);
	struct win_u_point_d wupd = {
		.args = args,
		.scale_function = scale_function
	};
	glfwSetWindowUserPointer(window, &wupd);

	set_callbacks(window);
	
	glClearColor(1.f, 1.f, 1.f, 0.f);
	glfwMainLoop(window, render_function, args);

	free(args);
	glfwDestroyWindow(window);
	glfwTerminate();
	return EXIT_SUCCESS;
}


static GLFWwindow *create_window(int width, int height, char fullscreen)
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
	struct win_u_point_d *wupd = (struct win_u_point_d *)glfwGetWindowUserPointer(window);
	wupd->scale_function(width, height, wupd->args);
}

static void set_callbacks(GLFWwindow *window)
{
	glfwSetKeyCallback(window, key_pressed_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
}

static void glfwMainLoop(GLFWwindow *window, render_f_t render_function, void *arguments)
{
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);
		render_function(arguments);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void GLAPIENTRY gl_error_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	fprintf(stderr, "[GL]: %s type = 0x%x, severity = 0x%x, id = 0x%x%s%s\n",
			(type == GL_DEBUG_TYPE_ERROR ? "[ERROR]: " : ""), type, severity, id, (length != 0 ? ", message = " : ""), message);
}
