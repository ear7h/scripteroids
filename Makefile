
SHELL := /usr/bin/env sh

UNAME := $(shell uname)
HEADLESS ?= false

CC := gcc

CFLAGS := -std=c99 \
	-Wall -Wextra -Wpedantic \
	-Wstrict-overflow

ifeq ($(HEADLESS), true)
	CFLAGS += -DHEADLESS -DRAYMATH_HEADER_ONLY
endif

INCLUDES := -I./include \
	-I./libvm/include/ \
	-I./vendor/raylib/src/ \
	-I./vendor/libdatastructure/include/


LD := gcc

ifeq ($(UNAME), Darwin)
	LDFLAGS := -framework CoreVideo \
		-framework IOKit \
		-framework Cocoa \
		-framework GLUT \
		-framework OpenGL
endif

ifeq ($(UNAME), Linux)
	LDFLAGS :=
endif


DEBUG := true

ifeq ($(DEBUG), true)
	CFLAGS += -g
else
	CFLAGS += -Werror
endif

PLATFORM=DESKTOP

ifeq ($(PLATFORM),DESKTOP)
	BUILD_DIR := build/$(shell uname)
else
	BUILD_DIR := build/$(shell echo $(PLATFORM) | tr '[:upper:]' '[:lower:]')
endif

$(BUILD_DIR):
	echo build dir is $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/libraylib.a:
	$(MAKE) -C ./vendor/raylib/src PLATFORM=PLATFORM_$(PLATFORM)
	cp ./vendor/raylib/src/libraylib.a $(BUILD_DIR)

$(BUILD_DIR)/libdatastructure.a:
	$(MAKE) -C ./vendor/libdatastructure archive
	cp ./vendor/libdatastructure/src/libdatastructure.a $(BUILD_DIR)


.PHONY: $(BUILD_DIR)/libvm.a
$(BUILD_DIR)/libvm.a:
	cd libvm && cargo build
	cp libvm/target/debug/libvm.a $(BUILD_DIR)

_LIBS = datastructure vm
ifneq ($(HEADLESS), true)
	_LIBS += raylib
endif

LIBS = $(_LIBS:%=$(BUILD_DIR)/lib%.a)

ifeq ($(UNAME), Linux)
	LINK_LIBS := -lpthread -ldl -lm
endif


.PHONY: libs
libs: $(BUILD_DIR) $(LIBS)
	echo $(LIBS)

$(BUILD_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

_OBJECTS = main rockets asteroids input lib
OBJECTS = $(_OBJECTS:%=$(BUILD_DIR)/%.o)

.PHONY: objects
objects: $(OBJECTS)

$(BUILD_DIR)/scripteroids: $(BUILD_DIR) $(LIBS) $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) $(LIBS) $(LINK_LIBS) -o $@

.PHONY: all
all: $(BUILD_DIR)/scripteroids

.PHONY: run
run: all
	./$(BUILD_DIR)/scripteroids

.PHONY: debug
debug: all
	lldb ./$(BUILD_DIR)/scripteroids

.PHONY: clean
clean:
	# $(MAKE) -C ./vendor/raylib/src clean
	rm -rf build

