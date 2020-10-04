#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <raylib.h>
#include <raymath.h>
#include <datastructure/slice.h>

#define NUM_ASTEROID_TYPES 4
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450
#define ASTEROID_TYPE_0 asteroids_types[0]
#define ASTEROID_TYPE_1 asteroids_types[1]
#define ASTEROID_TYPE_2 asteroids_types[2]
#define ASTEROID_TYPE_3 asteroids_types[3]

Vector2 rand_unit_vector2();
float rand_unit_float();

Vector2 center = {
	SCREEN_WIDTH/2,
	SCREEN_HEIGHT/2,
};

typedef struct asteroid_type {
	float radius;
	uint8_t child_count;
	uint8_t child_idx; // index into asteoroid_types
} asteroid_type_t;

struct asteroid_type asteroid_types[] = {
	{
		.radius = 10,
		.child_count = 0,
		.child_idx = 0,
	},
	{
		.radius = 15,
		.child_count = 0,
		.child_idx = 0,
	},
	{
		.radius = 30,
		.child_count = 2,
		.child_idx = 1,
	},
	{
		.radius = 40,
		.child_count = 2,
		.child_idx = 2,
	},
};

typedef struct asteroid {
	Vector2 pos;
	Vector2 dir;
	float rot;
	float rotdt;
	uint8_t type_idx;
} asteroid_t;


typedef struct asteroids {
	slice_t inner;
} asteroids_t;

asteroids_t asteroids_new() {
	slice_t inner = slice_new(sizeof(asteroid_t), 8, 16);

	for (size_t i = 0; i < 8; i++) {
		*(asteroid_t *) slice_idx(inner, i) = (struct asteroid) {
			.pos = center,
			.dir = rand_unit_vector2(),
			.rot = 360 * rand_unit_float(),
			.rotdt = 2 * rand_unit_float() - 1,
			.type_idx = 3,
		};
	}

	return (asteroids_t){
		.inner = inner,
	};
}

void asteroids_reset(asteroids_t * as) {
	slice_t inner = as->inner;
	inner = slice_slice(inner, 0, 0);
}

void asteroids_free(asteroids_t * as) {
	slice_free(&as->inner);
}

asteroid_t * asteroids_idx(asteroids_t as, size_t idx) {
	return (asteroid_t *) slice_idx(as.inner, idx);
}

size_t asteroids_len(asteroids_t as) {
	return slice_len(as.inner);
}

Vector2 vec_up = { .x = 0, .y = 1 };

#define SHIP_ANG_START (90.f)
#define SHIP_SIDE_LEN (30.f)
#define SQRT_3 (1.732050807568877f)
#define SHIP_OUT_CIRCLE (SHIP_SIDE_LEN / SQRT_3)
#define SHIP_IN_CIRCLE (SHIP_OUT_CIRCLE / 2.f)
// equilteral triangle of SHIP_SIDE_LEN
Vector2 ship_triangle[] = {
	{
		.y = 0,
		.x = SHIP_OUT_CIRCLE,
	},
	{
		.y = SHIP_SIDE_LEN / 2.f,
		.x = -SHIP_IN_CIRCLE,
	},
	{
		.y = -SHIP_SIDE_LEN / 2.f,
		.x = -SHIP_IN_CIRCLE,
	},
};

Vector2 Vector2FromPolar(float ang, float mag) {
	ang = ang*(PI/180.0f);
	return (Vector2) {mag * cosf(ang), mag * sin(ang) };
}

typedef struct ship {
	Vector2 pos;
	Vector2 dir;
	float ang;
	float acc;
} ship_t;


bool did_ship_collide(ship_t * ship, asteroid_t * a) {
	float dist = SHIP_OUT_CIRCLE + asteroid_types[a->type_idx].radius;
	return Vector2Distance(ship->pos, a->pos) <= dist;
}

bool did_point_collide(Vector2 point, asteroid_t * a) {
	float dist = asteroid_types[a->type_idx].radius;
	return Vector2Distance(point, a->pos) <= dist;
}


int main(void) {
	/*
	const int screenWidth = 800;
	const int screenHeight = 450;
	*/

	srand(10);

	asteroids_t asteroids = asteroids_new();
	ship_t ship = (ship_t) {
		.pos = center,
		.dir = {0, 0},
		.ang = SHIP_ANG_START,
		.acc = 0,
	};

	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "scripteroids");

	SetTargetFPS(60);

	bool game_over = false;

	// Detect window close button or ESC key
	while (!WindowShouldClose()) {

		if (IsKeyDown(KEY_Q)) break;

		BeginDrawing();

		ClearBackground(BLACK);

		bool ship_collision = false;

		// draw + move asteroids
		for (size_t i = 0; i < asteroids_len(asteroids); i++) {

			asteroid_t * a = asteroids_idx(asteroids, i);
			asteroid_type_t at = asteroid_types[a->type_idx];

			ship_collision |= did_ship_collide(&ship, a);

			// draw asteroid
			DrawPolyLines(a->pos, 6, at.radius, a->rot, WHITE);

			// move the asteroid
			a->rot = fmodf(360 + a->rot + a->rotdt, 360);
			a->pos = Vector2Add(
				a->pos,
				Vector2Scale(a->dir, 1));

			a->pos.x = fmodf(a->pos.x + SCREEN_WIDTH, SCREEN_WIDTH);
			a->pos.y = fmodf(a->pos.y + SCREEN_HEIGHT, SCREEN_HEIGHT);
		}

		// draw ship
		DrawTriangleLines(
			Vector2Add(
				Vector2Rotate(ship_triangle[0], ship.ang),
				ship.pos),
			Vector2Add(
				Vector2Rotate(ship_triangle[1], ship.ang),
				ship.pos),
			Vector2Add(
				Vector2Rotate(ship_triangle[2], ship.ang),
				ship.pos),
				WHITE);

		ship.pos = Vector2Add(ship.pos, ship.dir);

		ship.pos.x = fmodf(ship.pos.x + SCREEN_WIDTH, SCREEN_WIDTH);
		ship.pos.y = fmodf(ship.pos.y + SCREEN_HEIGHT, SCREEN_HEIGHT);

		// move the ship
		const float torque = 8;
		float omega = 0;
		if (IsKeyDown(KEY_A)) omega -= torque;
		if (IsKeyDown(KEY_D)) omega += torque;
		// printf("omega: %f", omega);
		ship.ang = fmodf(360 + ship.ang + omega, 360);

		ship.dir = Vector2Scale(ship.dir, 0.95);

		if (IsKeyDown(KEY_W)) {
			ship.acc = Clamp(ship.acc + 0.01, 0, 0.2);
		} else {
			ship.acc = 0;
		}

		ship.dir = Vector2Add(ship.dir,
			Vector2FromPolar(ship.ang, ship.acc));

		DrawFPS(10, 10);

		//ship.mag = Clamp(ship.mag + acc, 0, 6);
		static char buf[256];
		sprintf(buf, "%.2f", ship.ang);

		DrawText(buf, 10, 40, 20, WHITE);

		if (ship_collision) {
			game_over |= ship_collision;
			DrawText("collision: true" , 10, 60, 20, WHITE);
		} else {
			DrawText("collision: false" , 10, 60, 20, WHITE);
		}



		EndDrawing();
	}

	asteroids_free(&asteroids);

	// Close window and OpenGL context
	CloseWindow();

	return 0;
}

inline float rand_unit_float() {
	return ((float)rand()) / ((float)(RAND_MAX));
}

Vector2 rand_unit_vector2() {
	float theta = 2*PI * rand_unit_float();

	return (Vector2) { cosf(theta), sinf(theta) };
}
