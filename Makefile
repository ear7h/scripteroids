
SHELL := /bin/dash

CC := gcc

CFLAGS := -std=c99 \
	-Wall -Wextra -Wpedantic \
	-Wstrict-overflow \
	-I./include \
	-I./vendor/raylib/src/

LD := gcc

LDFLAGS := -framework CoreVideo \
	-framework IOKit \
	-framework Cocoa \
	-framework GLUT \
	-framework OpenGL

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
	

_LIBS = raylib
LIBS = $(_LIBS:%=$(BUILD_DIR)/lib%.a)

.PHONY: libs
libs: $(BUILD_DIR) $(LIBS)

$(BUILD_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

_OBJECTS = main
OBJECTS = $(_OBJECTS:%=$(BUILD_DIR)/%.o)

.PHONY: objects
objects: $(OBJECTS)

$(BUILD_DIR)/scripteroids: $(BUILD_DIR) $(LIBS) $(OBJECTS)
	$(LD) $(LDFLAGS) $(LIBS) $(OBJECTS) -o $@

.PHONY: all
all: $(BUILD_DIR)/scripteroids

.PHONY: run
run: all
	./$(BUILD_DIR)/scripteroids

.PHONY: clean
clean: 
	# $(MAKE) -C ./vendor/raylib/src clean
	rm -rf build

