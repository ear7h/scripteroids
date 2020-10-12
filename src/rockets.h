#pragma once

#include <datastructure/slice.h>
#include <raylib.h>


typedef struct rocket {
    Vector2 pos;
    float ang;
    bool valid;
} rocket_t;

typedef struct rockets {
	slice_t inner;
} rockets_t;

rockets_t rockets_new();

size_t rockets_len(rockets_t rockets);
rocket_t * rockets_idx(rockets_t rockets, size_t idx);
void rockets_free(rockets_t * rockets);

void rockets_add(rockets_t * rockets, rocket_t);
