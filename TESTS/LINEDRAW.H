#include "src/graphics.h"
#include "src/input.h"
#include "src/math.h"
#include "src/timer.h"
#include "src/utils.h"

// Bresenham line drawing test
void testBresenham(int ox, int oy, int r)
{
    double l;
    uint32_t startTime, numLines = 0;

    utl_printf(NULL, 1, 1, 15, 0, "Bresenham drawing: please wait...");

    startTime = tmr_getMs();

    for(l = 0.0; l < 2.0*M_PI; l += 0.0001, ++numLines)
    {
        int color = (int)((l * 10) + 1) % 256;
        gfx_drawLine(ox, oy, 0.f, ROUND(ox + r * cos(l)), ROUND(oy + r * sin(l)), 0.f, color, NULL);
    }

    utl_printf(NULL, 1, 1, 15, 0, "Bresenham drawing: %ums (%u lines)", tmr_getMs() - startTime, numLines);

    // exit on ESC press
    while(!kbd_keyPressed(KEY_ESC));
}
