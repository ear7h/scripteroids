# a really good Makefile by ear7h
# TODO: static library

# make variables

# explicitly set the shell to bash
SHELL := /bin/bash

# explicitly set the c compiler to gcc
CC := gcc

# use c11, give all warnings, more warnings, use the include dir in this repo
CFLAGS := -std=c11 -Wall -pedantic -I./include

ifeq ($(DEBUG),true)
	# if DEBUG=true (debug/dev build) write debug info into the binary
	CFLAGS += -g
else
	# if DEBUG=false (release build) treat warnings as errors
	CFLAGS += -Werror -Wno-error=unused-function
endif

# flags for test files
CTESTFLAGS :=-g -I./

# command that archives object files into a static library
AR := libtool -static

# libraries to be added to the binary
LIBS :=

# a debugger to run tests on, leave empty to run tests without
# a debugger (this value can be set in the command line as DB=...)
# potential values:
#		gdb --args
#		valgrind [--leak-check=full]
#		lldb --
# note that the this must be set so that the program and arguments can come
# right after the expansion of `$(DB)`. This variable is used
# like `$(DB) prog arg1 arg2 ...`
DB :=

# wildcard rules

# compile unit test source files; named xxx.test.c (and become xxx.test)
%.test.o: %.test.c
	$(CC) $(CFLAGS) $(CTESTFLAGS) -o $@ -c $<

# compile regular source files
%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

# compile tests
%.test: %.test.o %.o ./test/main.test.o
	$(CC) -o $@ $^ $(LIBS)

# phony rules

.PHONY: test clean

# run a test in 2 steps:

# put debug information in the compilation units
# note that this will not work when the compilation unit does
# not need to be recompiled (use -B to force compilation).
test::
	$(eval CFLAGS += -g)

# run the tests
test:: $(T)
	$(DB) ./$(T) '$(R)'

_OBJECTS = iterator list slice
OBJECTS = $(_OBJECTS:%=src/%.o)

.PHONY: objects
objects: $(OBJECTS)

src/libdatastructure.a: $(OBJECTS)
	$(AR) $(OBJECTS) -o $@

.PHONY: archive
archive: src/libdatastructure.a

# remove build artifacts
clean:
	rm -rf ./test/*.{o,dSYM} ./src/*.{o,test,dSYM}

