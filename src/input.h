#pragma once


typedef struct toggle {
    bool value;
    bool prev;
} toggle_t;

void toggle_update(toggle_t * state, bool current_down);
bool toggle_value(toggle_t * state);

typedef struct every {
    float every;
    float last_time;
    bool current;
} every_t;

void every_update(every_t * state, bool current_down, float current_time);
bool every_value(every_t * state);


