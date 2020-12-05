#include <eglib.h>
#include <eglib/display.h>
#include "draw_test.h"

void draw_test(eglib_t *eglib) {
  coordinate_t x, y;

  x = 50 - 20;
  y = 50 + 14 / 2;

  eglib_SetIndexColor(eglib, 0, 0, 0, 128);
  eglib_DrawLine(eglib, 0, y, 99, y);
  eglib_DrawLine(eglib, x, 0, x, 99);

  eglib_SetFont(eglib, &font_LiberationSans_Regular_20px);
  eglib_AddUnicodeBlockToFont(
    &font_LiberationSans_Regular_20px,
    &unicode_block_LiberationSans_Regular_20px_Latin1Supplement
  );
  eglib_SetIndexColor(eglib, 0, 255, 255, 255);
  eglib_DrawText(eglib, x, y, "Olá!");
}