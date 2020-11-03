#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#include <shaders.h>

/*	INIT	0x0
 *	SHFO	0x1	shader file opened
 *	SHCR	0x2 shader code read
 *	SHCC	0x4	shader code compiled
 */
typedef enum {INIT, SHFO, SHCR = 0x2, SHCC = 0x4} cleanup_flags;
typedef enum {PROGRAM, SHADER} sec_type;

static GLenum detect_type(const char *);
static char *read_shader(const char *, GLenum *);
static int check_shader_errors(GLuint, sec_type);

GLuint create_shader_program(int n, char * const shader_paths[])
{
	GLuint shader[n];
	char *shader_code;
	int i, error = 0;
	GLenum shader_type;
	GLuint program;
	uint_fast8_t flags = INIT;

	program = glCreateProgram();
	for (i = 0; i < n; ++i) {
		shader[i] = 0;
		if (!(shader_code = read_shader(shader_paths[i], &shader_type))) {
			fprintf(stderr, "[ERROR]: reading %s\n", shader_paths[i]);
			error = 1;
			goto cleanup;
		}
		flags |= SHCR;
		shader[i] = glCreateShader(shader_type);
		glShaderSource(shader[i], 1, (const GLchar * const *)&shader_code, NULL);
		glCompileShader(shader[i]);
		if(check_shader_errors(shader[i], SHADER)) {
			fprintf(stderr, "[ERROR]: %s compile error\n", shader_paths[i]);
			shader[i] = 0;
			error = 1;
			goto cleanup;
		}
		flags |= SHCC;
		glAttachShader(program, shader[i]);
		free(shader_code);
		flags &= ~SHCR;
	}

	glLinkProgram(program);
	if (check_shader_errors(program, PROGRAM)) {
		fprintf(stderr, "[ERROR]: program link error\n");
		program = 0;
		error = 1;
	}

cleanup:
	if (flags & SHCC) for (i = 0; i < n; ++i) glDeleteShader(shader[i]);
	if (flags & SHCR) free(shader_code);
	if (error && program != 0) {
		glDeleteProgram(program);
		program = 0;
	}
	return program;
}

static char *read_shader(const char *shader_path, GLenum *shader_type)
{
	FILE *shader_file;
	char *shader_string = NULL;
	struct stat st;
	off_t size;
	uint_fast8_t flags = INIT;
	int error = 0;
	
	if (GL_INVALID_VALUE == (*shader_type = detect_type(shader_path))) {
		error = 1;
		goto cleanup;
	}
	if (stat(shader_path, &st)) {
		fprintf(stderr, "[ERROR]: couldn't read stat of file %s\n", shader_path);
		error = 1; 
		goto cleanup;
	}
	if (!(shader_file = fopen(shader_path, "r"))) {
		fprintf(stderr, "[ERROR]: couldn't open file %s\n", shader_path);
		error = 1;
		goto cleanup;
	}
	flags |= SHFO;

	size = st.st_size;
	
	shader_string = calloc(size + 1, sizeof(*shader_string));
	if (!shader_string) {
		fprintf(stderr, "[ERROR]: couldn't allocate memory for shader %s\n", shader_path);
		error = 1;
		goto cleanup;
	}

	for (int i = 0; i < size; ++i) {
		if (EOF == (shader_string[i] = fgetc(shader_file))) {
			fprintf(stderr, "[ERROR]: couldn't read byte %d from %s\n", i + 1, shader_path);
			error = 1;
			goto cleanup;
		}
	}
	
cleanup:
	if (flags & SHFO) fclose(shader_file);
	if (error) {
		if (shader_string) {
			free(shader_string);
			shader_string = NULL;
		}
	}
	return shader_string;
}

static GLenum detect_type(const char *shader_path)
{
	if (strrchr(shader_path, '.') == NULL) {
		fprintf(stderr, "[ERROR]: %s: no extension\n", shader_path);
		errno = EINVAL;
		return GL_INVALID_VALUE;
	} else {
		const char *ext = strrchr(shader_path, '.');
		if (!strcmp(".vert", ext)) {
			return GL_VERTEX_SHADER;
		} else if (!strcmp(".tesc", ext)) {
			return GL_TESS_CONTROL_SHADER;
		} else if (!strcmp(".tese", ext)) {
			return GL_TESS_EVALUATION_SHADER;
		} else if (!strcmp(".geom", ext)) {
			return GL_GEOMETRY_SHADER;
		} else if (!strcmp(".frag", ext)) {
			return GL_FRAGMENT_SHADER;
		} else if (!strcmp(".comp", ext)) {
			return GL_COMPUTE_SHADER;
		}
	}
	fprintf(stderr, "[ERROR]: %s: invalid file extension\n", shader_path);
	errno = EINVAL;
	return GL_INVALID_VALUE;
}

static int check_shader_errors(GLuint object, sec_type type)
{
	GLint no_error = 0;
	switch (type) {
	case SHADER:
		glGetShaderiv(object, GL_COMPILE_STATUS, &no_error);
		if (no_error == GL_FALSE) {
			GLint length = 0;
			glGetShaderiv(object, GL_INFO_LOG_LENGTH, &length);
			GLchar buffer[length];
			glGetShaderInfoLog(object, length, NULL, buffer);
			glDeleteShader(object);

			fprintf(stderr, "[ERROR]: shader: %s", buffer);
			errno = EINVAL;
		}
		break;
	case PROGRAM:
		glGetProgramiv(object, GL_LINK_STATUS, &no_error);
		if (no_error == GL_FALSE) {
			GLint length = 0;
			glGetProgramiv(object, GL_INFO_LOG_LENGTH, &length);
			GLchar buffer[length];
			glGetProgramInfoLog(object, length, NULL, buffer);
			glDeleteProgram(object);

			fprintf(stderr, "[ERROR]: program: %s", buffer);
			errno = EINVAL;
		}
		break;
	}
	return !(no_error == GL_TRUE);
}
