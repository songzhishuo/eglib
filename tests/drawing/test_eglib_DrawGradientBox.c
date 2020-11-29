#include <eglib.h>
#include <eglib/display.h>
#include "draw_test.h"

void draw_test(eglib_t *eglib) {
  eglib_SetIndexColor(eglib, 0, 255, 0, 0);
  eglib_SetIndexColor(eglib, 1, 0, 255, 0);
  eglib_SetIndexColor(eglib, 2, 0, 0, 255);
  eglib_SetIndexColor(eglib, 3, 255, 255, 255);
  eglib_DrawGradientBox(eglib, 10, 10, 80, 80);
}