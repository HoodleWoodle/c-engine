#ifndef ENGINE_H
#define ENGINE_H

#include <stdbool.h>
#include <stdint.h>
#include "key.h"

// ################################################################
// defines
// ################################################################

#define NUMBER_OF_CELLS_ON_AXIS_X 80 // 320
#define NUMBER_OF_CELLS_ON_AXIS_Y 40 // 240
#define PIXELS_PER_CELL           16 //   5
#define PIXEL_GAP_BETWEEN_CELLS    1 //   0

#define BLACK 0x000000
#define WHITE 0xFFFFFF
#define RED   0xFF0000
#define GREEN 0x00FF00
#define BLUE  0x0000FF

#define COLOR_BACKGROUND_DEFAULT 0x252525
#define COLOR_CELL_DEFAULT       0xDDDDDD

// ################################################################
// types
// ################################################################

typedef uint32_t u32;
typedef int32_t i32;
typedef uint32_t color_t;

typedef enum {
    KEY_STATE_DOWN,     // last frame: <key-pressed>,     current frame: <key-pressed>
    KEY_STATE_UP,       // last frame: <key-not-pressed>, current frame: <key-not-pressed>
    KEY_STATE_PRESSED,  // last frame: <key-not-pressed>, current frame: <key-pressed>
    KEY_STATE_RELEASED, // last frame: <key-pressed>,     current frame: <key-not-pressed>
} key_state_t;

typedef int key_id_t; // key_t already exists :(

// ################################################################
// function declarations
// ################################################################

void engine_init();
void engine_deinit();

bool engine_is_running();
void engine_frame_begin();
void engine_frame_end();

void engine_set_background(color_t);
void engine_set_cell(i32 x, i32 y, color_t);
color_t engine_get_cell(i32 x, i32 y);

key_state_t engine_get_key_state(key_id_t);

#endif // ENGINE_H