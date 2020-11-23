dir		:= $(realpath ./)

source	:= src
target	:= build

dev			:= $(source)/dev
inc			:= $(source)/inc
test_source	:= $(source)/test

bin			:= $(target)/bin
obj			:= $(target)/obj
test_target	:= $(target)/test

source_dirs := $(dev) $(inc) $(test_source)
target_dirs	:= $(bin) $(obj) $(test_target)

simulation_object_files := $(addsuffix .o,$(addprefix $(obj)/,particle config timing random colors simulation))
plotter_object_files := $(addsuffix .o,$(addprefix $(obj)/,drawing shaders colors model utils))

CC		:= gcc
CFLAGS	:= -I $(inc) -Wall -Wpedantic -Wextra -std=gnu99 -O3 -fdiagnostics-color=always
SLIBS	:= $(shell pkg-config --libs gsl)
GLIBS	:= $(shell pkg-config --libs glew glfw3) -lm

.PHONY : all test clean mostlyclean

.PHONY : echo tree target objects 

.PHONY : color_test shader_test model_test simulation drawing

all : test simulation plotter
test : color_test shader_test model_test

color_test : $(test_target)/color_test
shader_test : $(test_target)/shader_test
model_test : $(test_target)/model_test
simulation : $(bin)/simulation 
plotter : $(bin)/shaders $(bin)/plotter


$(test_target)/color_test : $(test_source)/color_test.c $(inc)/particle.h | $(test_target)
	$(CC) $(CFLAGS) $< -o $@

$(test_target)/shader_test : $(test_source)/shader_test.c $(inc)/shaders.h $(obj)/shaders.o | $(test_target)
	$(CC) $(CFLAGS) $(GLIBS) $< $(obj)/shaders.o -o $@

$(test_target)/model_test : $(test_source)/model_test.c $(inc)/model.h | $(test_target)
	$(CC) $(CFLAGS) $(GLIBS) $< -o $@

$(test_target)/drawing_test : $(test_source)/drawing_test.c $(addsuffix .o,$(addprefix $(obj)/,drawing shaders colors utils)) | $(test_target)
	$(CC) $(CFLAGS) $(GLIBS) $< $(addsuffix .o,$(addprefix $(obj)/,drawing shaders colors model utils)) -o $@


objects : $(simulation_object_files) $(plotter_object_files)
$(simulation_object_files) $(plotter_object_files) : | $(obj)


$(obj)/particle.o : $(dev)/particle.c $(inc)/particle.h $(inc)/config.h $(inc)/random.h $(inc)/colors.h
	$(CC) $(CFLAGS) -c $< -o $@

$(obj)/config.o : $(dev)/config.c $(inc)/config.h
	$(CC) $(CFLAGS) -c $< -o $@

$(obj)/timing.o : $(dev)/timing.c $(inc)/timing.h
	$(CC) $(CFLAGS) -c $< -o $@

$(obj)/random.o : $(dev)/random.c $(inc)/random.h
	$(CC) $(CFLAGS) -c $< -o $@

$(obj)/simulation.o : $(dev)/simulation.c $(inc)/config.h $(inc)/particle.h
	$(CC) $(CFLAGS) -c $< -o $@


$(obj)/drawing.o : $(dev)/drawing.c $(inc)/drawing.h
	$(CC) $(CFLAGS) -c $< -o $@

$(obj)/shaders.o : $(dev)/shaders.c $(inc)/shaders.h 
	$(CC) $(CFLAGS) -c $< -o $@

$(obj)/colors.o : $(dev)/colors.c $(inc)/colors.h 
	$(CC) $(CFLAGS) -c $< -o $@

$(obj)/model.o : $(dev)/model.c $(inc)/model.h $(inc)/shaders.h 
	$(CC) $(CFLAGS) -c $< -o $@

$(obj)/utils.o : $(dev)/utils.c $(inc)/utils.h 
	$(CC) $(CFLAGS) -c $< -o $@


$(bin)/simulation : $(simulation_object_files) | $(bin)
	$(CC) $(CFLAGS) $(SLIBS) $(simulation_object_files) -o $@ 

$(bin)/plotter : $(plotter_object_files) | $(bin)
	$(CC) $(CFLAGS) $(GLIBS) $(plotter_object_files) -o $@


target : | $(target_dirs)

$(target) : 
	mkdir $(target)

$(target_dirs) : | $(target)
	mkdir $@

$(bin)/shaders : $(source)/shaders | $(bin)
	cp -r $(source)/shaders $(bin)


clean :
	-rm -rf $(target)

mostlyclean :
	-rm -rf $(target)/obj

tree : 
	@tree -I 'from_alibal'

echo : 
	@echo variable info
	$(info $(source_dirs))
	$(info $(target_dirs))

