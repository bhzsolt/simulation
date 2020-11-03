#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <shaders.h>

typedef int (*test_function)(int, char * const []);

int test_0(int, char * const []);
int test_1(int, char * const []);

GLFWwindow *create_window(int, int, char);
void error_callback(int, const char *);

int main(int argc, char * const argv[])
{
	test_function jtable[] = {test_0, test_1};
	int retval = EXIT_SUCCESS;
	
	int to_run = atoi(argv[1]);
	int n, i;
	switch (to_run) {
	case 0:
		n = sizeof(jtable)/sizeof(test_function);
		for (i = 0; i < n; ++i) {	
			if (jtable[i](argc - 2, argv + 2) != EXIT_SUCCESS) {
				printf("errors in test %d\n", i + 1);
				retval = EXIT_FAILURE;
			}
		}
		break;
	default:
		if (to_run < 1) exit(EXIT_FAILURE);
		i = to_run - 1;
		if (jtable[i](argc - 2, argv + 2) != EXIT_SUCCESS) {
			printf("errors in test %d\n", i + 1);
			retval = EXIT_FAILURE;
		}
		break;
	}
	exit(retval);
}

int test_0(int argc, char * const argv[])
{
	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) exit(EXIT_FAILURE);

	GLFWwindow *window;
	if (!(window = create_window(600, 600, 0))) {
		glfwTerminate();
		return EXIT_FAILURE;
	}
	glewExperimental = GL_TRUE;
	glewInit();
	GLint program = create_shader_program(argc, argv);
	if (!program) {
		GLint length = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
		if (length != 0) {
			GLchar buffer[length];
			glGetProgramInfoLog(program, length, NULL, buffer);
			printf("info log:\n%s\n", buffer);
		}
		perror("create shaders");
		return EXIT_FAILURE;
	}

	GLint shader_count = 0;
	glGetProgramiv(program, GL_ATTACHED_SHADERS, &shader_count);
	printf("%d attached shaders\n", shader_count);

	glDeleteProgram(program);
	glfwDestroyWindow(window);
	glfwTerminate();
	return EXIT_SUCCESS;
}
int test_1(int argc, char * const argv[])
{
	int flags = 0x2;
	int errorvalue = 1;

#define check_errors(error, string) do {if (errorvalue) { printf("%d %s\n", (error), (string));} } while (0)
	check_errors(flags, "first try");
#undef check_errors
#define check_errors(error, string) do {if (errorvalue) { fprintf(stderr, "[ERROR]: %s\n", (string)); flags |= (error); goto cleanup;}} while (0)
	check_errors(0x1, "this is the actual checker");
	printf("hi\n");
#undef check_errors

cleanup:
	flags &= 0x1;
	return flags;
}

GLFWwindow *create_window(int width, int height, char fullscreen)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	GLFWwindow *window;
	if (!(window = glfwCreateWindow(width, height, "", NULL, NULL))) return NULL;
	glfwMakeContextCurrent(window);
	glfwSetWindowSizeLimits(window, 200, 200, 1000, 1000);
	glfwSetWindowAspectRatio(window, 1, 1);
	glfwSwapInterval(1);
	return window;
}

void error_callback(int error, const char *description)
{
	fprintf(stderr, "[ERROR]: %s\n", description);
}
