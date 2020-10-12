#pragma once

#include <stdlib.h>
#include <math.h>

#include <raymath.h>

static inline float rand_unit_float() {
	return ((float)rand()) / ((float)(RAND_MAX));
}

static inline Vector2 rand_unit_vector2() {
	float theta = 2*PI * rand_unit_float();

	return (Vector2) { cosf(theta), sinf(theta) };
}
