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

object_files := $(addsuffix .o,$(addprefix $(obj)/,particle config timing random colors simulation))

CC		:= gcc
CFLAGS	:= -I $(inc) -Wall -Wpedantic -Wextra -std=gnu99 -O3 -fdiagnostics-color=always
SLIBS	:= $(shell pkg-config --libs gsl)
GLIBS	:= $(shell pkg-config --libs glew glfw3) -lm

.PHONY : all test clean mostlyclean

.PHONY : echo tree target objects 

.PHONY : color_test simulation drawing

all : test simulation drawing
test : color_test shader_test

color_test : $(test_target)/color_test
shader_test : $(test_target)/shader_test
simulation : $(bin)/simulation
drawing : $(bin)/drawing


$(test_target)/color_test : $(test_source)/color_test.c $(inc)/particle.h | $(test_target)
	$(CC) $(CFLAGS) $< -o $@

$(test_target)/shader_test : $(test_source)/shader_test.c $(inc)/shaders.h $(obj)/shaders.o | $(test_target)
	$(CC) $(CFLAGS) $(GLIBS) $< $(obj)/shaders.o -o $@

objects : $(object_files) $(obj)/drawing.o
$(object_files) : | $(obj)

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

$(obj)/drawing.o : $(dev)/drawing.c $(inc)/shaders.h $(inc)/colors.h | $(obj)
	$(CC) $(CFLAGS) -c $< -o $@

$(obj)/shaders.o : $(dev)/shaders.c $(inc)/shaders.h | $(obj)
	$(CC) $(CFLAGS) -c $< -o $@

$(obj)/colors.o : $(dev)/colors.c $(inc)/colors.h | $(obj)
	$(CC) $(CFLAGS) -c $< -o $@


$(bin)/simulation : $(object_files) | $(bin)
	$(CC) $(CFLAGS) $(SLIBS) $(object_files) -o $@ 

$(bin)/drawing : $(obj)/drawing.o $(obj)/shaders.o $(obj)/colors.o
	$(CC) $(CFLAGS) $(GLIBS) $< $(obj)/shaders.o $(obj)/colors.o -o $@


target : | $(target_dirs)

$(target) : 
	mkdir $(target)

$(target_dirs) : | $(target)
	mkdir $@



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

