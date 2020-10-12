#include <assert.h>

#include "utils.h"
#include "asteroids.h"


asteroid_type_t asteroid_types[] = {
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

asteroids_t asteroids_new() {
	slice_t inner = slice_new(sizeof(asteroid_t), 0, 16);

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

asteroid_t * asteroids_data(asteroids_t as) {
	return slice_data(as.inner);
}

void asteroids_init(asteroids_t * as, float width, float height) {

	for (size_t i = 0; i < 8; i++) {
		bool is_x = rand_unit_float() < 0.5;
		bool is_origin = rand_unit_float() < 0.5;

		float x = width * rand_unit_float() * ((float) is_x) +
			((float) !is_x) * ((float) !is_origin) * (width-1);
		float y = height * rand_unit_float() * ((float) !is_x) +
			((float) is_x) * ((float) !is_origin) * (width-1);

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

void asteroids_add(asteroids_t * as, asteroid_t a) {
    assert(a.valid);

    for (size_t i = 0; i < asteroids_len(*as); i++) {
        asteroid_t * aa = asteroids_idx(*as, i);
        if (!aa->valid) {
            *aa = a;
            return;
        }
    }

    as->inner = slice_append(as->inner, &a);
}

