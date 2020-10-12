#pragma once

#include <stdint.h>

#include "../../src/asteroids.h"

typedef struct vm vm_t;
// typedef struct asteroid asteroid_t;

vm_t * vm_c_compile(const char *);
void vm_c_free(const vm_t * vm);
uint8_t vm_c_step(vm_t * vm, asteroid_rel_t * asteroids, size_t len);

