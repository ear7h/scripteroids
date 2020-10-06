#pragma once

#include <raylib.h>
#include <stdint.h>
#include <datastructure/slice.h>


typedef struct asteroid_type {
	float radius;
	uint8_t child_count;
	uint8_t child_idx; // index into asteoroid_types
} asteroid_type_t;

extern asteroid_type_t asteroid_types[];

typedef struct asteroid {
	Vector2 pos;
	Vector2 dir;
	float rot;
	float rotdt;
	uint8_t type_idx;
	bool valid;
} asteroid_t;


typedef struct asteroids {
	slice_t inner;
} asteroids_t;

asteroids_t asteroids_new();
void asteroids_reset(asteroids_t * as);
void asteroids_free(asteroids_t * as);

asteroid_t * asteroids_idx(asteroids_t as, size_t idx);
size_t asteroids_len(asteroids_t as);
asteroid_t * asteroids_data(asteroids_t as);

void asteroids_add(asteroids_t * as, asteroid_t a);
