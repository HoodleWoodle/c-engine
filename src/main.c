#include <stdio.h>
#include "engine.h"

i32 clamp(i32 min, i32 max, i32 v) {
	return (v < min) ? min : ((v > max) ? max : v);
}

int main() {
	i32 x = 0, y = 0;

	engine_init();
	while (engine_is_running()) {
		engine_frame_begin();

		// code
		engine_set_cell(x, y, GREEN);
		if (engine_get_key_state(KEY_W) == KEY_STATE_DOWN) y--;
		if (engine_get_key_state(KEY_S) == KEY_STATE_DOWN) y++;
		if (engine_get_key_state(KEY_A) == KEY_STATE_DOWN) x--;
		if (engine_get_key_state(KEY_D) == KEY_STATE_DOWN) x++;
		x = clamp(0, NUMBER_OF_CELLS_ON_AXIS_X - 1, x);
		y = clamp(0, NUMBER_OF_CELLS_ON_AXIS_Y - 1, y);
		engine_set_cell(x, y, RED);

		engine_frame_end();
	}
	engine_deinit();
	return 0;
}
