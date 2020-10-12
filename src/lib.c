#include <math.h>

#include <raymath.h>
#include <datastructure/slice.h>

#include "lib.h"

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



bool did_ship_collide(ship_t * ship, asteroid_t * a) {
	float dist = SHIP_OUT_CIRCLE + asteroid_types[a->type_idx].radius;
	return Vector2Distance(ship->pos, a->pos) <= dist;
}

bool did_rocket_collide(rocket_t * r, asteroid_t * a) {
	float dist = asteroid_types[a->type_idx].radius;
	return Vector2Distance(r->pos, a->pos) <= dist;
}

bool scripteroids_step(scripteroids_t * s, uint8_t ship_control) {
	s->steps++;
	bool ship_collision = false;

	// draw + move asteroids
	for (size_t i = 0; i < asteroids_len(s->asteroids); i++) {
		asteroid_t * a = asteroids_idx(s->asteroids, i);

		if (!a->valid) {
			continue;
		}

		for (size_t i = 0; i < rockets_len(s->rockets); i++) {
			rocket_t * r = rockets_idx(s->rockets, i);

			if (!r->valid) {
				continue;
			}

			if (did_rocket_collide(r, a)) {
				a->valid = false;
				r->valid = false;
				s->score++;
				goto asteroid_loop;
			}
		}

		ship_collision |= did_ship_collide(&s->ship, a);

		// move the asteroid
		a->rot = fmodf(360 + a->rot + a->rotdt, 360);
		a->pos = Vector2Add(a->pos, a->dir);

		a->pos.x = fmodf(a->pos.x + SCREEN_WIDTH, SCREEN_WIDTH);
		a->pos.y = fmodf(a->pos.y + SCREEN_HEIGHT, SCREEN_HEIGHT);

asteroid_loop: {};

	}

	for (size_t i = 0; i < rockets_len(s->rockets); i++) {
		rocket_t * r = rockets_idx(s->rockets, i);

		if (!r->valid) {
			continue;
		}

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

	// fire rockets
	every_update(&s->rocket_every,
			ship_control & SHIP_CONTROL_FIRE,
			((float) s->steps) / 60.0);

	if (every_value(&s->rocket_every)) {
		// front of the ship
		Vector2 pos = Vector2Add(
				Vector2Rotate(ship_triangle[0], s->ship.ang),
				s->ship.pos);

		rocket_t r = (rocket_t) {
			.pos = pos,
				.ang = s->ship.ang,
				.valid = true,
		};

		rockets_add(&s->rockets, r);
	}

	// move the ship
	const float torque = 8;
	float omega = 0;

	if (ship_control & SHIP_CONTROL_LEFT) omega -= torque;
	if (ship_control & SHIP_CONTROL_RIGHT) omega += torque;
	// printf("omega: %f", omega);
	s->ship.ang = fmodf(0 + s->ship.ang + omega, 360);

	s->ship.dir = Vector2Scale(s->ship.dir, 0.95);

	if (ship_control & SHIP_CONTROL_FORWARD) {
		s->ship.acc = Clamp(s->ship.acc + 0.01, 0, 0.2);
	} else {
		s->ship.acc = 0;
	}

	s->ship.dir = Vector2Add(s->ship.dir,
			Vector2FromPolar(s->ship.ang, s->ship.acc));

	s->ship.pos = Vector2Add(s->ship.pos, s->ship.dir);

	s->ship.pos.x = fmodf(s->ship.pos.x + SCREEN_WIDTH, SCREEN_WIDTH);
	s->ship.pos.y = fmodf(s->ship.pos.y + SCREEN_HEIGHT, SCREEN_HEIGHT);

	return ship_collision;
}



void scripteroids_asteroids_rel(const scripteroids_t * s,
		asteroid_rel_t ** ars,
		size_t * len,
		size_t * cap) {

	slice_t slc = (slice_t) {
		.elsize = sizeof(asteroid_rel_t),
		.len = 0,
		.cap = *cap,
		.data = *ars,
	};

	ship_t ship = s->ship;
	asteroids_t as = s->asteroids;

	for (size_t i = 0; i < asteroids_len(as); i++) {
		asteroid_t a = *asteroids_idx(as, i);
		if (!a.valid) {
			continue;
		}

		float angle = ship.ang - Vector2Angle(
				ship_forward_basis,
				Vector2Subtract(a.pos, ship.pos));

		if (angle > 180.0) { angle -= 360.0; }

		// the Vector2Angle function is incorrect
		float heading;
		{
			Vector2 a2s = Vector2Subtract(s->ship.pos, a.pos);
			float dot = Vector2DotProduct(a2s, a.dir);
			float det = a2s.x * a.dir.y - a2s.y * a.dir.x;
			heading = (180.0f/PI) * -atan2f(det, dot);
		}
		if (heading > 180.0) { heading -= 360; }

		asteroid_rel_t ar = (asteroid_rel_t) {
			.radius = a.radius,
			.distance = Vector2Distance(a.pos, ship.pos),
			.angle = angle,
			.heading = heading,
		};

		slc = slice_append(slc, &ar);
	}

	*ars = slc.data;
	*len = slc.len;
	*cap = slc.cap;

}
