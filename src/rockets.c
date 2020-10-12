#include <assert.h>

#include "rockets.h"

rockets_t rockets_new() {
    return (rockets_t) {
        .inner = slice_new(sizeof(rocket_t), 0, 16),
    };
}

size_t rockets_len(rockets_t rs) {
	return slice_len(rs.inner);
}

rocket_t * rockets_idx(rockets_t rs, size_t idx) {
	return slice_idx(rs.inner, idx);
}

void rockets_free(rockets_t * rs) {
	slice_free(&rs->inner);
}


void rockets_add(rockets_t * rs, rocket_t r) {
    assert(r.valid);

    for (size_t i = 0; i < rockets_len(*rs); i++) {
        rocket_t * rr = rockets_idx(*rs, i);
        if (!rr->valid) {
            *rr = r;
            return;
        }
    }

    rs->inner = slice_append(rs->inner, &r);
}
