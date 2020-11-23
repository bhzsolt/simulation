#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <model.h>
#include <shaders.h>

struct model {
	const char *name;
	struct uniform *uniforms;
	size_t n_uniform;
	GLuint shader_program;
	GLuint vao;
	GLuint vbo;
	GLenum mode;
	GLsizei count;
};

struct uniform {
	const char *name;
	GLint id;
	uniform_type type;
	GLsizei count;
	GLboolean transpose;
	void *data;
};

static struct uniform *init_uniforms(GLuint, size_t, struct uniform_specs *);
static int set_attribute(struct model *, struct attribute *);
static void gl_uniform_wrapper(struct uniform *, void *);

struct model *create_model(struct model_specs *specs)
{
	{
#if defined INIT || defined ERROR || defined MODEL || defined VAO
#	error "flag already defined"
#else
#	define	INIT	0x00
#	define	ERROR	0x01	
#	define	MODEL	0x02
#	define	VAO		0x04
#	define	VBO		0x08
#	define	SHPR	0x10
#	define	UNIF	0x12
#	define	ATTR	0x14
#endif
#if defined CHECK_ERROR || defined CHECK_GL_ERROR 
#	error "error check already defined"
#else
#	define CHECK_ERROR(value, string) do {if ((value)) {\
	fprintf(stderr, "[ERROR]: model %s setup: %s\n", specs->name, (string));\
	perror("\t"); \
	flags |= ERROR; goto exit;};} while (0)
#	define CHECK_GL_ERROR(string) do {if (GL_NO_ERROR != glGetError()) { \
	fprintf(stderr, "[ERROR]: [GL]: model %s setup: %s\n", specs->name, (string)); flags |= ERROR; goto exit;}\
	;} while (0)
#endif
	}
	int flags = INIT;
	register size_t i = 0;
	
	struct model *model = calloc(1, sizeof *model);
	CHECK_ERROR(!model, "allocating model");
	flags |= MODEL;

	model->name = specs->name;
	
	glGenVertexArrays(1, &(model->vao));
	glBindVertexArray(model->vao);
	//printf("%s vao: %d\n", model->name, model->vao);
	CHECK_GL_ERROR("binding vertex array object");
	flags |= VAO;

	glGenBuffers(1, &(model->vbo));
	glBindBuffer(GL_ARRAY_BUFFER, model->vbo);
	glBufferData(GL_ARRAY_BUFFER, specs->size, specs->data, specs->usage);
	CHECK_GL_ERROR("binding vertex buffer object");
	flags |= VBO;

	model->shader_program = create_shader_program(specs->n_shader, specs->shader_source);
	CHECK_ERROR(!model->shader_program, "creating shader program");
	glUseProgram(model->shader_program);
	CHECK_GL_ERROR("using shader program");
	flags |= SHPR;

	/*for (i = 0; i < specs->n_attribute; ++i) {
		CHECK_ERROR(set_attribute(model, specs->attributes + i), "setting attributes");
		flags |= ATTR;
	}*/

	glVertexAttribPointer(glGetAttribLocation(model->shader_program, "position"), 
			2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0
			);
	glEnableVertexAttribArray(glGetAttribLocation(model->shader_program, "position"));
	glVertexAttribPointer(glGetAttribLocation(model->shader_program, "color"), 
			1, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const void *) (2 * sizeof(GLfloat))
			);
	glEnableVertexAttribArray(glGetAttribLocation(model->shader_program, "color"));

	model->n_uniform = specs->n_uniform;
	model->uniforms = init_uniforms(model->shader_program, specs->n_uniform, specs->uniforms);
	CHECK_ERROR(!model->uniforms, "initializing uniforms");
	flags |= UNIF;

	model->mode = specs->mode;
	model->count = specs->n_points;

exit:
	if (flags & ERROR) {
		if (flags & UNIF) {
			free(model->uniforms);
			model->uniforms = NULL;
		}
		if (flags & SHPR) glDeleteProgram(model->shader_program);
		if (flags & VBO) glDeleteBuffers(1, &(model->vbo));
		if (flags & VAO) glDeleteVertexArrays(1, &(model->vao));
		if (flags & MODEL) {
			free(model);
			model = NULL;
		}
	}
	if (flags & VAO) glBindVertexArray(0);
	return model;
	{
#undef CHECK_GL_ERROR
#undef CHECK_ERROR
#undef ATTR
#undef UNIF
#undef SHPR
#undef VBO
#undef VAO
#undef MODEL
#undef ERROR
#undef INIT
	}
}

void cleanup_model(struct model *model)
{
	free(model->uniforms);
	glDeleteVertexArrays(1, &(model->vao));
	glDeleteBuffers(1, &(model->vbo));
	glDeleteProgram(model->shader_program);
	free(model);
}

int draw_model(struct model *model)
{
	{
#ifdef CHECK_GL_ERROR
#	error "error check already defined"
#else
#	define CHECK_GL_ERROR(string) do {if (GL_NO_ERROR != glGetError()) {\
	fprintf(stderr, "[ERROR]: draw model %s: %s\n", model->name, (string));\
	return 1; \
	}} while (0)
#endif
	}
	glUseProgram(model->shader_program);
	set_uniform("colors", NULL, model);
	CHECK_GL_ERROR("use shader program");
	glBindVertexArray(model->vao);
	//printf("%s vao: %d\n", model->name, model->vao);
	CHECK_GL_ERROR("bind vertex array");
	glDrawArrays(model->mode, 0, model->count);
	CHECK_GL_ERROR("draw");
	return 0;
	{
#undef CHECK_GL_ERROR
	}
}

int set_uniform(const char *uni_name, void *data, struct model *model)
{
	{
#ifdef CHECK_ERROR
#	error "error check already defined"
#else
#	define CHECK_ERROR(value, string) \
	if ((value)) { \
		fprintf(stderr, "[ERROR]: model %s: set_uniform: %s: %s\n", model->name, uni_name, (string)); \
		return 1;\
	}
#endif
	}
	register size_t i = 0;
	register int found = 0;
	while (!found && (i < model->n_uniform)) {
		if (!strcmp(model->uniforms[i].name, uni_name)) {
			found = 1;
			glUseProgram(model->shader_program);
			if (data) {
				model.uniform[i].data = data;
			}
			gl_uniform_wrapper(model->uniforms + i, model->uniforms[i].data);
		} else {
			++i;
		}
	}
	CHECK_ERROR(i >= model->n_uniform, "no such uniform");
	CHECK_ERROR(GL_NO_ERROR != glGetError(), "error setting value");
	return 0;
	{
#undef CHECK_ERROR
	}
}


void add_attribute(const char *name, GLint size, GLenum type, GLsizei stride, const void *offset, struct attribute *attribute)
{
	attribute->name = name;
	attribute->pointer = offset;
	attribute->size = size;
	attribute->type = type;
	attribute->stride = stride;
	attribute->normalized = GL_FALSE;
}


void add_float_uniform(const char *name, void *data, struct uniform_specs *uniform)
{
	uniform->name = name;
	uniform->data = data;
	uniform->type = U_1F;
}

void add_int_uniform(const char *name, void *data, struct uniform_specs *uniform)
{
	uniform->name = name;
	uniform->data = data;
	uniform->type = U_1I;
}

void add_vect3_uniform(const char *name, void *data, GLsizei count, struct uniform_specs *uniform)
{
	uniform->name = name;
	uniform->data = data;
	uniform->type = U_3FV;
	uniform->count = count;
}

void add_matrix_uniform(const char *name, void *data, struct uniform_specs *uniform)
{
	uniform->name = name;
	uniform->data = data;
	uniform->type = U_M4FV;
	uniform->count = 1;
	uniform->transpose = GL_TRUE;
}


static void gl_uniform_wrapper(struct uniform *uniform, void *data)
{
	switch (uniform->type) {
	case U_1F:
		glUniform1f(uniform->id, *((float *)data));
		break;
	case U_1I:
		glUniform1i(uniform->id, *((int *)data));
		break;
	case U_3FV:
		glUniform3fv(uniform->id, uniform->count, data);
		break;
	case U_M4FV:
		glUniformMatrix4fv(uniform->id, uniform->count, uniform->transpose, data);
		break;
	}
}

static struct uniform *init_uniforms(GLuint program, size_t n, struct uniform_specs *specs)
{
	{
#if defined INIT || defined ERROR
#	error "flag already defined"
#else
#	define	INIT	0x00
#	define	ERROR	0x01
#	define	UNIF	0x02
#endif
#if defined CHECK_ERROR || defined CHECK_GL_ERROR 
#	error "error check already defined"
#else
#	define CHECK_ERROR(value, string) do {if ((value)) {\
	fprintf(stderr, "[ERROR]: uniform setup: %s\n", (string));\
	perror("\t"); \
	flags |= ERROR; goto exit;};} while (0)
#	define CHECK_GL_ERROR(string) do {if (GL_NO_ERROR != glGetError()) { \
	fprintf(stderr, "[ERROR]: [GL]: uniform %s: %s\n", specs[i].name, (string)); flags |= ERROR; goto exit;}\
	;} while (0)
#endif
	}
	register int flags = INIT;
	struct uniform *uniforms = calloc(n, sizeof *uniforms);
	CHECK_ERROR(!uniforms, "allocating uniforms");
	flags |= UNIF;
	
	register size_t i;
	for (i = 0; i < n; ++i) {
		uniforms[i].name = specs[i].name;
		uniforms[i].type = specs[i].type;
		uniforms[i].count = specs[i].count;
		uniforms[i].transpose = specs[i].transpose;
		uniforms[i].data = specs[i].data;
		uniforms[i].id = glGetUniformLocation(program, specs[i].name);
		CHECK_GL_ERROR("error getting location");

		gl_uniform_wrapper(uniforms + i, specs[i].data);
		CHECK_GL_ERROR("error setting value");
	}
	
exit:
	if (flags & ERROR && flags & UNIF) {
		free(uniforms);
		uniforms = NULL;
	}
	return uniforms;
	{
#undef ERROR
#undef INIT
#undef CHECK_GL_ERROR
#undef CHECK_ERROR
	}
}

static int set_attribute(struct model *model, struct attribute *attribute)
{
	{
#if defined INIT || defined ERROR || defined POS || defined SET || defined ENAB
#	error "flag already defined"
#else
#	define	INIT	0x00
#	define	ERROR	0x01
#endif
#if defined CHECK_ATTRIBUTE || defined CHECK_ERROR
#	error "error check already defined"
#else
#	define CHECK_ATTRIBUTE(name) do {if ((attr_id) == -1) { \
	fprintf(stderr, "[ERROR]: %s: no such attribute\n", (name)); \
	flags |= ERROR; goto exit;}; \
	} while (0)
#	define CHECK_GL_ERROR(action) do {if (GL_NO_ERROR != glGetError()) { \
	fprintf(stderr, "[ERROR]: %s %s attribute\n", (action), attribute->name); \
	flags |= ERROR; goto exit;}; \
	} while (0)
#endif
	}
	register int flags = INIT;
	register GLint attr_id;

	glBindVertexArray(model->vao);
	glBindBuffer(GL_ARRAY_BUFFER, model->vbo);
	attr_id = glGetAttribLocation(model->shader_program, attribute->name);
	CHECK_ATTRIBUTE(attribute->name);
	CHECK_GL_ERROR("getting");

	glVertexAttribPointer(attr_id, 
						  attribute->size, 
						  attribute->type, 
						  attribute->normalized,
						  attribute->stride,
						  attribute->pointer);
	CHECK_GL_ERROR("setting");

	glEnableVertexAttribArray(attr_id);
	CHECK_GL_ERROR("enabling");
	
exit:
	return flags & ERROR;
	{
#undef ERROR
#undef INIT
#undef CHECK_ATTRIBUTE
#undef CHECK_ERROR
	}
}
