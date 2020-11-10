#ifndef __MODEL_H__
#define __MODEL_H__

#include <GL/glew.h>

typedef enum {U_1F, U_1I, U_3FV, U_M4FV} uniform_type;

struct uniform_specs {
	const char *name; //uniform name
	void *data; //uniform data
	uniform_type type; //uniform type
	GLsizei count;
	GLboolean transpose;
};

struct attribute {
	const char *name; //attribute name
	const void *pointer; //offset of the first component
	GLint size; //number of components per attribute
	GLenum type; //GL_FLOAT, etc
	GLsizei stride; //byte offset between vertex attributes. 0 for tightly packed
	GLboolean normalized; //GL_FALSE (can be GL_TRUE when glVertexAttribPointer with integer type)
};

struct model_specs {
	GLsizeiptr size; //sizeof() * (color + coord) * n 
	const void *data; //pointer to data
	const char *name; //model name
	struct attribute *attributes;
	struct uniform_specs *uniforms;
	char * const *shader_source; // = (char * const []){"","",""};
	size_t n_attribute;
	size_t n_uniform;
	size_t n_shader;
	size_t n_points;
	GLenum usage; // GL_STREAM_DRAW || GL_STATIC_DRAW
	GLenum mode; //GL_POINTS || GL_LINE_STRIP || ...
};

struct uniform;
struct model;
struct model *create_model(struct model_specs *);
void cleanup_model(struct model *);
int draw_model(struct model *);
int set_uniform(const char *, void *, struct model *);

void add_attribute(const char *, GLint, GLenum, GLsizei, const void *, struct attribute *);

void add_float_uniform(const char *, void *, struct uniform_specs *);
void add_int_uniform(const char *, void *, struct uniform_specs *);
void add_vect3_uniform(const char *, void *, GLsizei count, struct uniform_specs *);
void add_matrix_uniform(const char *, void *, struct uniform_specs *);


#endif /* model.h */

