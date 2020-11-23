#include <stdio.h>

#include <GL/glew.h>
#include <GL/gl.h>

#include <drawing.h>
#include <shaders.h>
#include <colors.h>
#include <utils.h>

struct setup_data {
	GLfloat *coords;
	GLfloat *colors;
	GLuint program;
	GLsizei count;
};

struct render_data {
	GLuint program;
	GLint smat_id;
	GLsizei count;
};

void *setup_function(void *);
void render_function(void *);
void scale_function(int, int, void *);

int main(int argc, char * const argv[])
{
	GLfloat coords[] = {0.0, 0.0, BLUE};
	GLfloat *colors = create_colors(5);
	GLuint program = create_shader_program(argc - 1, argv + 1);
	
	struct setup_data data = {
		.coords = &coords,
		.colors = colors,
		.program = program,
		.count = 1
	};

	drawing(setup_function, render_function, scale_function, &data, 0);

	free(colors);
	return 0;
}

void *setup_function(void *args)
{
	struct setup_data *data = (struct data *)args;
	size_t size = sizeof *(data->coords) * 3;
	size_t stride = sizeof *(data->coords) * 2;

	glUseProgram(data->program);

	GLuint vao, vbo;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, size, data->coords, GL_STATIC_DRAW);

	GLint pos = glGetAttribLocation(data->program, "position");
	GLint col = glGetAttribLocation(data->program, "color");

	glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, size, 0);
	glVertexAttribPointer(col, 1, GL_FLOAT, GL_FALSE, size, (const void *) stride);

	glEnableVertexAttribArray(pos);
	glEnableVertexAttribArray(col);

	GLfloat *rmat = create_rotation_matrix(36);
	GLfloat *smat = create_scale_matrix(1.0, WINDOW_X, WINDOW_Y);
	
	GLint id = glGetUniformLocation(data->program, "radius");
	glUniform1f(id, 0.1);
	id = glGetUniformLocation(data->program, "n_segments");
	glUniform1i(id, 36);
	id = glGetUniformLocation(data->program, "colors");
	glUniform3fv(id, 5, data->colors);
	id = glGetUniformLocation(data->program, "rotation_matrix");
	glUniformMatrix4fv(id, 1, GL_TRUE, rmat);
	id = glGetUniformLocation(data->program, "scale_matrix");
	glUniformMatrix4fv(id, 1, GL_TRUE, smat);
	
	struct render_data *out = calloc(1, sizeof *out);
	out->program = data->program;
	out->smat_id = id;
	out->count = data->count;

	free(rmat);
	free(smat);
	return out;
}

void render_function(void *args)
{
	struct render_data *data = (struct render_data *)args;
	//glUseProgram(data->program);
	glDrawArrays(GL_POINTS, 0, data->count);
}

void scale_function(int w, int h, void *args)
{
	struct render_data *data = (struct render_data *)args;
	GLfloat *smat = create_scale_matrix(1.0, w, h);
	glUniformMatrix4fv(data->smat_id, 1, GL_TRUE, smat);
	free(smat);
}
