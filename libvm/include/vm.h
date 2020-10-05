#pragma once

#include <stdint.h>


#define SHIP_CONTROL_FORWARD 1
#define SHIP_CONTROL_LEFT 2
#define SHIP_CONTROL_RIGHT 4

// typedef struct vm vm_t;
// typedef struct asteroid asteroid_t;

vm_t * vm_c_compile(uint8_t *);
void vm_c_free(const void *);
void vm_c_step(void *, void * ptr, size_t len);
uint8_t vm_c_get_ship_control(const void *);
