#include <math.h>
#include <stdio.h>
#include <stdint.h>

#include <raylib.h>
#include <raymath.h>
#include <datastructure/slice.h>
#include <vm.h>

#include "utils.h"
#include "lib.h"



Vector2 center = {
	SCREEN_WIDTH/2,
	SCREEN_HEIGHT/2,
};

int main(void) {
	srand(10);

	scripteroids_t s = (scripteroids_t) {
		.asteroids = asteroids_new(),
		.rockets = rockets_new(),
		.rocket_every = (every_t) {
			.every = 1.0 / 10.0,
			.last_time = 0.0,
			.current = false,
		},
		.ship = (ship_t) {
			.pos = center,
			.dir = {0, 0},
			.ang = SHIP_ANG_START,
			.acc = 0,
		},
		.steps = 0,
	};

	asteroids_init(&s.asteroids, SCREEN_WIDTH, SCREEN_HEIGHT);



	slice_t asteroid_rel_buf = {
		.elsize = sizeof(asteroid_rel_t),
		.len = 0,
		.cap = 0,
		.data = NULL,
	};

	vm_t * vm = vm_c_compile("");
	bool game_over = false;

#ifndef HEADLESS
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "scripteroids");

	SetTargetFPS(60);

	// Detect window close button or ESC key
	while (!WindowShouldClose()) {

		if (IsKeyDown(KEY_Q)) break;

		BeginDrawing();

		ClearBackground(BLACK);

		// draw asteroids
		for (size_t i = 0; i < asteroids_len(s.asteroids); i++) {
			asteroid_t * a = asteroids_idx(s.asteroids, i);

			if (!a->valid) {
				continue;
			}

			asteroid_type_t at = asteroid_types[a->type_idx];

			// draw asteroid
			DrawPolyLines(a->pos, 6, at.radius, a->rot, WHITE);
		}


		for (size_t i = 0; i < rockets_len(s.rockets); i++) {
			rocket_t * r = rockets_idx(s.rockets, i);

			if (!r->valid) {
				continue;
			}

			DrawCircle(r->pos.x, r->pos.y, 1.0, WHITE);
		}


		// draw ship
		if (!game_over) {
			DrawTriangleLines(
				Vector2Add(
					Vector2Rotate(ship_triangle[0], s.ship.ang),
					s.ship.pos),
				Vector2Add(
					Vector2Rotate(ship_triangle[1], s.ship.ang),
					s.ship.pos),
				Vector2Add(
					Vector2Rotate(ship_triangle[2], s.ship.ang),
					s.ship.pos),
					WHITE);
		}

		scripteroids_asteroids_rel(&s,
			&asteroid_rel_buf.data,
			&asteroid_rel_buf.len,
			&asteroid_rel_buf.cap);

		for (size_t i = 0; i < slice_len(asteroid_rel_buf); i++) {
			asteroid_rel_t ar = *(asteroid_rel_t *) slice_idx(asteroid_rel_buf, i);


			Vector2 endv = Vector2Rotate(
				ship_forward_basis,
				s.ship.ang - ar.angle);

			Vector2 end = Vector2Add(
				s.ship.pos,
				Vector2Scale(endv, 0.1 * ar.distance));

			DrawLineV(s.ship.pos, end, GREEN);
			DrawCircleV(end, 3.0, GREEN);

			size_t j = 0;
			size_t k = 0;
			while (true) {
				asteroid_t a = *asteroids_idx(s.asteroids, j);
				if (!a.valid) {
					j++;
					continue;
				}

				if (k == i) {

					Vector2 endv  = Vector2Rotate(a.dir, ar.heading);
					Vector2 end = Vector2Add(
						a.pos,
						Vector2Scale(endv, 0.1 * ar.distance));

					DrawLineV(a.pos, end, RED);
					DrawCircleV(end, 3.0, RED);

					break;
				}

				j++;
				k++;
			}
		}
#else
	while (true) {
#endif

		// get commands from the VM
		uint8_t ship_control = vm_c_step(vm,
			asteroid_rel_buf.data,
			asteroid_rel_buf.len);


#ifndef HEADLESS
		// get commands from keyboard
		if (IsKeyDown(KEY_SPACE)) { ship_control |= SHIP_CONTROL_FIRE; }
		if (IsKeyDown(KEY_A)) { ship_control |= SHIP_CONTROL_LEFT; }
		if (IsKeyDown(KEY_D)) { ship_control |= SHIP_CONTROL_RIGHT; }
		if (IsKeyDown(KEY_W)) { ship_control |= SHIP_CONTROL_FORWARD; }
#endif

		// run game tick
		bool ship_collision = scripteroids_step(&s, ship_control);

#ifndef HEADLESS

		static char buf[256];

		DrawFPS(10, 10);

		//ship.mag = Clamp(ship.mag + acc, 0, 6);
		// static char buf[256];
		sprintf(buf, "%.2f", s.ship.ang);

		DrawText(buf, 10, 40, 20, WHITE);

		if (ship_collision) {
			game_over |= ship_collision;
			DrawText("collision: true" , 10, 60, 20, WHITE);
		} else {
			DrawText("collision: false" , 10, 60, 20, WHITE);
		}

		EndDrawing();
#else
		if (ship_collision) {
			game_over = true;
			break;
		}
#endif
	}

	asteroids_free(&s.asteroids);
	rockets_free(&s.rockets);
	vm_c_free(vm);
	slice_free(&asteroid_rel_buf);

#ifndef HEADLESS
	// Close window and OpenGL context
	CloseWindow();
#endif

	if (game_over) {
		return 1;
	} else {
		return 0;
	}
}

