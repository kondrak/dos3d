#include "src/graphics.h"
#include "src/input.h"
#include "src/math.h"
#include "src/timer.h"

// Bresenham line drawing test
void testBresenham(int ox, int oy, int r)
{
    float l;
    unsigned long int startTime, numLines = 0;
    const unsigned short *keysPressed = kbd_getInput();

    tmr_start();

    fprintf(stdout, "Bresenham drawing: please wait...\r");
    fflush(stdout);

    startTime = tmr_getMs();

    for(l = 0.0f; l < 2.0f*M_PI; l += 0.001f, ++numLines)
    {
        int color = (int)((l * 10.f) + 1) % 256;
        gfx_drawLine(ox, oy, 0.f, ox + r * cos(l), oy + r * sin(l), 0.f, color, NULL);
    }

    fprintf(stdout, "Bresenham drawing: %ums (%u lines)", tmr_getMs() - startTime, numLines);
    fflush(stdout);
    
    while(!keysPressed[KEY_ESC]) keysPressed = kbd_getInput();

    tmr_finish();
}
