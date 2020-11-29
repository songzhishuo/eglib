#include <eglib.h>
#include <eglib/display.h>
#include "draw_test.h"

void draw_test(eglib_t *eglib) {
	eglib_SetIndexColor(eglib, 0, 0, 255, 0);
	eglib_DrawPixelColor(
		eglib,
		10, 10,
		(color_t){
			.r = 0,
			.g = 255,
			.b = 0,
		}
	);
}