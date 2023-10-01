#ifndef ENGINE_H
#define ENGINE_H

#include <stdbool.h>
#include <stdint.h>
#include "input_def.h"

// ################################################################
// defines
// ################################################################

#define VSYNC 0 // 0: off, 1: on

#define NUMBER_OF_CELLS_ON_AXIS_X 320 // 320
#define NUMBER_OF_CELLS_ON_AXIS_Y 240 // 240
#define PIXELS_PER_CELL             5 //   5
#define PIXEL_GAP_BETWEEN_CELLS     0 //   0

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
    INPUT_STATE_DOWN,     // last frame: <pressed>,     current frame: <pressed>
    INPUT_STATE_UP,       // last frame: <not-pressed>, current frame: <not-pressed>
    INPUT_STATE_PRESSED,  // last frame: <not-pressed>, current frame: <pressed>
    INPUT_STATE_RELEASED, // last frame: <pressed>,     current frame: <not-pressed>
} input_state_t;

typedef int key_id_t; // key_t already exists :(
typedef int button_id_t; // key_t already exists :(

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

input_state_t engine_get_key_state(key_id_t);
input_state_t engine_get_button_state(button_id_t);
bool engine_get_hovered_cell(i32* x, i32* y);

#endif // ENGINE_H