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

object_files := $(addsuffix .o,$(addprefix $(obj)/,particle config timing random simulation))

CC		= gcc
CFLAGS	= -I $(inc) -Wall -Wpedantic -Wextra -std=gnu99 -O3 -fdiagnostics-color=always
LIBS	= -lm -lgsl -lgslcblas

.PHONY : all test clean mostlyclean

.PHONY : echo tree

.PHONY : color_test target objects simulation

all : test simulation
simulation : $(bin)/simulation
test : color_test

color_test : $(test_target)/color_test

$(test_target)/color_test : $(test_source)/color_test.c $(inc)/particle.h
	$(CC) $(CFLAGS) $< -o $@

objects : $(object_files)
$(object_files) : | $(obj)

$(obj)/particle.o : $(dev)/particle.c $(inc)/particle.h $(inc)/config.h $(inc)/random.h
	$(CC) $(CFLAGS) -c $< -o $@

$(obj)/config.o : $(dev)/config.c $(inc)/config.h
	$(CC) $(CFLAGS) -c $< -o $@

$(obj)/timing.o : $(dev)/timing.c $(inc)/timing.h
	$(CC) $(CFLAGS) -c $< -o $@

$(obj)/random.o : $(dev)/random.c $(inc)/random.h
	$(CC) $(CFLAGS) -c $< -o $@

$(obj)/simulation.o : $(dev)/simulation.c $(inc)/config.h $(inc)/particle.h
	$(CC) $(CFLAGS) -c $< -o $@


$(bin)/simulation : $(object_files) | $(bin)
	$(CC) $(CFLAGS) $(LIBS) $(object_files) -o $@ 


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

