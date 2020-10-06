#pragma once

#include <stdint.h>

#include "../../src/asteroids.h"

#define SHIP_CONTROL_FORWARD 1
#define SHIP_CONTROL_LEFT 2
#define SHIP_CONTROL_RIGHT 4
#define SHIP_CONTROL_FIRE 8

typedef struct vm vm_t;
// typedef struct asteroid asteroid_t;

vm_t * vm_c_compile(const char *);
void vm_c_free(const vm_t * vm);
void vm_c_step(vm_t * vm, asteroid_t * asteroids, size_t len);
uint8_t vm_c_get_ship_control(const void * vm);
