#include <stdio.h>

#include <model.h>

int main()
{

	struct attribute attribute;
	printf("attribute:\t\t%ld\n"
			"\tname:\t\t%ld\n"
			"\tpointer:\t%ld\n"
			"\tsize:\t\t%ld\n"
			"\ttype:\t\t%ld\n"
			"\tstride:\t\t%ld\n"
			"\tnormalized:\t%ld\n",
			sizeof(attribute), 
			sizeof(attribute.name), 
			sizeof(attribute.pointer),
			sizeof(attribute.size),
			sizeof(attribute.type),
			sizeof(attribute.stride),
			sizeof(attribute.normalized)
			);

	struct model_specs model_s;
	model_s.shader_source = (char * const []){"01", "02", "03"};
	printf("model_specs:\t\t%ld\n"
			"\tsize:\t\t%ld\n"
			"\tdata:\t\t%ld\n"
			"\tattributes:\t%ld\n"
			"\tshader_source:\t%ld\n"
			"\tusage:\t\t%ld\n",
			sizeof(model_s),
			sizeof(model_s.size),
			sizeof(model_s.data),
			sizeof(model_s.attributes),
			sizeof(model_s.shader_source),
			sizeof(model_s.usage)
			);


	printf("size_t:\t%ld\n"
			"int:\t%ld\n",
			sizeof(size_t),
			sizeof(int));

	return 0;
}
