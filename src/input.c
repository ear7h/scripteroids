
#include <stdint.h>
#include <stdbool.h>

#include "input.h"

void toggle_update(toggle_t * state, bool current_down) {
    if (state->prev && !current_down) {
        state->value = !state->value;
    }
}

bool toggle_value(toggle_t * state) {
    return state->value;
}

void every_update(every_t * state, bool current_down, float current_time) {
    if (state->every < current_time - state->last_time) {
        state->current = current_down;
        state->last_time = current_time;
    } else {
        state->current = false;
    }
}

bool every_value(every_t * state) {
    return state->current;
}

