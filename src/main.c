#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <raylib.h>
#include <raymath.h>
#include <datastructure/slice.h>
#include <vm.h>

#include "input.h"
#include "rockets.h"
#include "asteroids.h"


#define NUM_ASTEROID_TYPES 4
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450

Vector2 rand_unit_vector2();
float rand_unit_float();

Vector2 center = {
	SCREEN_WIDTH/2,
	SCREEN_HEIGHT/2,
};


void init_asteroids(asteroids_t * as) {

	for (size_t i = 0; i < 8; i++) {
		bool is_x = rand_unit_float() < 0.5;
		bool is_origin = rand_unit_float() < 0.5;

		float x = SCREEN_WIDTH * rand_unit_float() * ((float) is_x) +
			((float) !is_x) * ((float) !is_origin) * (SCREEN_WIDTH-1);
		float y = SCREEN_HEIGHT * rand_unit_float() * ((float) !is_x) +
			((float) is_x) * ((float) !is_origin) * (SCREEN_HEIGHT-1);

		Vector2 pos = (Vector2){
			.x = x,
			.y = y, };

		asteroid_t a = (asteroid_t) {
			.pos = pos,
			.dir = rand_unit_vector2(),
			.rot = 360 * rand_unit_float(),
			.rotdt = 2 * rand_unit_float() - 1,
			.type_idx = 3,
			.valid = true,
		};

		asteroids_add(as, a);
	}
}

Vector2 ship_forward_basis = { .y = 0, .x = 1 };

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

bool did_rocket_collide(rocket_t * r, asteroid_t * a) {
	float dist = asteroid_types[a->type_idx].radius;
	return Vector2Distance(r->pos, a->pos) <= dist;
}


int main(void) {
	/*
	const int screenWidth = 800;
	const int screenHeight = 450;
	*/

	srand(10);

	asteroids_t asteroids = asteroids_new();
	init_asteroids(&asteroids);

	rockets_t rockets = rockets_new();
	every_t rocket_every = (every_t) {
		.every = 1.0 / 10.0,
		.last_time = 0.0,
		.current = false,
	};

	ship_t ship = (ship_t) {
		.pos = center,
		.dir = {0, 0},
		.ang = SHIP_ANG_START,
		.acc = 0,
	};
	vm_t * vm = vm_c_compile("");

	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "scripteroids");

	SetTargetFPS(60);

	bool game_over = false;

	// Detect window close button or ESC key
	while (!WindowShouldClose()) {

		if (IsKeyDown(KEY_Q)) break;

		vm_c_step(vm,
			asteroids_data(asteroids),
			asteroids_len(asteroids));

		BeginDrawing();

		ClearBackground(BLACK);

		bool ship_collision = false;

		// draw + move asteroids
		for (size_t i = 0; i < asteroids_len(asteroids); i++) {
			asteroid_t * a = asteroids_idx(asteroids, i);

			if (!a->valid) {
				continue;
			}

			for (size_t i = 0; i < rockets_len(rockets); i++) {
				rocket_t * r = rockets_idx(rockets, i);

				if (!r->valid) {
					continue;
				}

				if (did_rocket_collide(r, a)) {
					a->valid = false;
					r->valid = false;
					goto asteroid_loop;
				}
			}

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

asteroid_loop: {};

		}

		for (size_t i = 0; i < rockets_len(rockets); i++) {
			rocket_t * r = rockets_idx(rockets, i);

			if (!r->valid) {
				continue;
			}

			DrawCircle(r->pos.x, r->pos.y, 1.0, WHITE);
			// move asteroid
			r->pos = Vector2Add(
					r->pos,
					Vector2Scale(Vector2Rotate(ship_forward_basis, r->ang),
						5.0));
			float x = r->pos.x;
			float y = r->pos.y;

			if (x < 0 || x > SCREEN_WIDTH || y < 0 || y > SCREEN_WIDTH) {
				r->valid = false;
			}
		}

		// draw ship
		if (!game_over) {
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
		}


		uint8_t ship_control = vm_c_get_ship_control(vm);

		// fire rockets
		every_update(&rocket_every,
			IsKeyDown(KEY_SPACE) | (ship_control & SHIP_CONTROL_FIRE),
			GetTime());

		if (every_value(&rocket_every)) {
			// front of the ship
			Vector2 pos = Vector2Add(
				Vector2Rotate(ship_triangle[0], ship.ang),
				ship.pos);

			rocket_t r = (rocket_t) {
				.pos = pos,
				.ang = ship.ang,
				.valid = true,
			};

			rockets_add(&rockets, r);
		}

		if (!game_over) {
			// move the ship
			const float torque = 8;
			float omega = 0;
			if (IsKeyDown(KEY_A) | (ship_control & SHIP_CONTROL_LEFT)) omega -= torque;
			if (IsKeyDown(KEY_D) | (ship_control & SHIP_CONTROL_RIGHT)) omega += torque;
			// printf("omega: %f", omega);
			ship.ang = fmodf(360 + ship.ang + omega, 360);

			ship.dir = Vector2Scale(ship.dir, 0.95);

			if (IsKeyDown(KEY_W) | (ship_control & SHIP_CONTROL_FORWARD)) {
				ship.acc = Clamp(ship.acc + 0.01, 0, 0.2);
			} else {
				ship.acc = 0;
			}

			ship.dir = Vector2Add(ship.dir,
				Vector2FromPolar(ship.ang, ship.acc));

			ship.pos = Vector2Add(ship.pos, ship.dir);

			ship.pos.x = fmodf(ship.pos.x + SCREEN_WIDTH, SCREEN_WIDTH);
			ship.pos.y = fmodf(ship.pos.y + SCREEN_HEIGHT, SCREEN_HEIGHT);
		}

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
