#include <stdio.h>
#include <time.h>
#include "engine.h"

#define SPEED 16.0

float clamp(float min, float max, float v) {
	return (v < min) ? min : ((v > max) ? max : v);
}

int example(void) {
	float x = 0, y = 0;

	clock_t last = clock();

	engine_init();
	while (engine_is_running()) {
		engine_frame_begin();

		clock_t now = clock();
		float dt = ((float)(now - last) / (float)CLOCKS_PER_SEC) * 3.5;
		last = now;

		engine_set_cell((i32)x, (i32)y, GREEN);
		if (engine_get_key_state(KEY_W) == INPUT_STATE_DOWN) y -= SPEED * dt;
		if (engine_get_key_state(KEY_S) == INPUT_STATE_DOWN) y += SPEED * dt;
		if (engine_get_key_state(KEY_A) == INPUT_STATE_DOWN) x -= SPEED * dt;
		if (engine_get_key_state(KEY_D) == INPUT_STATE_DOWN) x += SPEED * dt;
		x = clamp(0, NUMBER_OF_CELLS_ON_AXIS_X - 1, x);
		y = clamp(0, NUMBER_OF_CELLS_ON_AXIS_Y - 1, y);
		engine_set_cell((i32)x, (i32)y, RED);

		if (engine_get_button_state(BUTTON_LEFT) == INPUT_STATE_DOWN) {
			i32 xh, yh;
			if (engine_get_hovered_cell(&xh, &yh)) {
				engine_set_cell(xh, yh, BLUE);
			}
		}

		engine_frame_end();
	}
	engine_deinit();
	return 0;
}
