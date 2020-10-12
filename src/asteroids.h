#pragma once

#include <raylib.h>
#include <stdint.h>
#include <datastructure/slice.h>

#define NUM_ASTEROID_TYPES 4

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
    float radius;
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
void asteroids_init(asteroids_t * as, float width, float height);

typedef struct asteroid_rel {
    // radius of the asteroid
    float radius;

    // distance from the ship
    float distance;

    // the angle of the asteroid relative the the ship's heading.
    // that is, 0 means the asteroid is in front of the ship.
    float angle;

    // asteroid heading relative to the ship.
    // that is, 0 means the ship is going towards the ship
    float heading;
} asteroid_rel_t;

