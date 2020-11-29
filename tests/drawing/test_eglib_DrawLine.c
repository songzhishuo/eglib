#include <eglib.h>
#include <eglib/display.h>
#include "draw_test.h"

void draw_test(eglib_t *eglib) {
	eglib_SetIndexColor(eglib, 0, 0, 255, 0);

	eglib_DrawLine(eglib, 0, 0, 99, 0);
	eglib_DrawLine(eglib, 0, 0, 50, 50);
	eglib_DrawLine(eglib, 0, 0, 0, 99);

	eglib_DrawLine(eglib, 99, 99, 99, 0);
	eglib_DrawLine(eglib, 99, 99, 50, 50);
	eglib_DrawLine(eglib, 99, 99, 0, 99);

	eglib_DrawLine(eglib, 0, 99, 50, 50);
	eglib_DrawLine(eglib, 99, 0, 50, 50);
}