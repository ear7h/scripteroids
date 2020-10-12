#pragma once

#include "input.h"
#include "rockets.h"
#include "asteroids.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450

#define SHIP_ANG_START (90.f)
#define SHIP_SIDE_LEN (30.f)
#define SQRT_3 (1.732050807568877f)
#define SHIP_OUT_CIRCLE (SHIP_SIDE_LEN / SQRT_3)
#define SHIP_IN_CIRCLE (SHIP_OUT_CIRCLE / 2.f)

#define SHIP_CONTROL_FORWARD 1
#define SHIP_CONTROL_LEFT 2
#define SHIP_CONTROL_RIGHT 4
#define SHIP_CONTROL_FIRE 8

// equilteral triangle of SHIP_SIDE_LEN
extern Vector2 ship_triangle[];

static Vector2 ship_forward_basis = { .y = 0, .x = 1 };

typedef struct ship {
	Vector2 pos;
	Vector2 dir;
	float ang;
	float acc;
} ship_t;

typedef struct scripteroids {
	asteroids_t asteroids;
	rockets_t rockets;
	every_t rocket_every;
	ship_t ship;
	uint64_t steps;
    uint64_t score;
} scripteroids_t;

bool scripteroids_step(scripteroids_t * s, uint8_t ship_control);

void scripteroids_asteroids_rel(const scripteroids_t * s,
    asteroid_rel_t ** ar,
    size_t * len,
    size_t * cap);
