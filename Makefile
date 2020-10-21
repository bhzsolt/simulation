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

object_files := $(addsuffix .o,$(addprefix $(obj)/,particle config))

CC		= gcc
CFLAGS	= -I $(inc) -Wall -Wpedantic -Wextra -std=gnu99 -O3 -fdiagnostics-color=always

.PHONY : all test clean mostlyclean

.PHONY : echo tree

.PHONY : color_test target objects

color_test : $(test_target)/color_test

$(test_target)/color_test : $(test_source)/color_test.c $(inc)/particle.h
	$(CC) $(CFLAGS) $< -o $@

objects : $(object_files)
$(object_files) : | $(obj)

$(obj)/particle.o : $(dev)/particle.c $(inc)/particle.h $(inc)/config.h
	$(CC) $(CFLAGS) -c $< -o $@

$(obj)/config.o : $(dev)/config.c $(inc)/config.h
	$(CC) $(CFLAGS) -c $< -o $@


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

