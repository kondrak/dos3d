#include "src/input.h"
#include "src/graphics.h"
#include "src/math.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static const int SCREEN_WIDTH  = 320;
static const int SCREEN_HEIGHT = 200;

// good bye
void Shutdown(int exitCode)
{
    setMode(0x03);
    exit(exitCode);
}

unsigned short *keysPressed;

void drawCircle(int ox, int oy, int r)
{
    float l = 0.0f;
    for(l; l < 2.0f*M_PI; l += 0.001f)
    {
      int color = (int)((l * 10.f) + 1) % 256;
      drawLine(ox, oy, ox + r * cos(l), oy + r * sin(l), color);
      drawPixel(ox + r * cos(l), oy + r * sin(l), color);
      keysPressed = translateInput();
      if(keysPressed[KEY_ESC]) return;
    }
}


/* ***** */

int main(int argc, char **argv)
{
    float invWidth, invHeight, aspectRatio, angle;
    int x, y, renderFinished = 0;
    int fov = 45;

    keysPressed = translateInput();
    
    for (x = 0; x < argc; x++)
    {
        // custom fov flag
        if (!strcmp(argv[x], "-f") && x + 1 < argc)
        {
            if (isdigit(argv[x + 1][0]))
                fov = atof(argv[x + 1]);
        }
    }

    setMode(0x13);

    // calculate the view
    invWidth = 1.0f / SCREEN_WIDTH;
    invHeight = 1.0f / SCREEN_HEIGHT;
    aspectRatio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
    angle = tan(M_PI * 0.5 * fov / 180.);

    while (!keysPressed[KEY_ESC])
    {
        // perform draw after vsync
        if (!renderFinished)
        {
            for (x = 0; x < SCREEN_WIDTH; x++)
            {
                for (y = 0; y < SCREEN_HEIGHT; y++)
                {
                    drawPixel(x, y, 0);
                }
            }

            drawCircle(160, 100, 95);
            
            renderFinished = 1;
            keysPressed = translateInput();
        }
        else
        {
            keysPressed = translateInput();
        }
    }
    
    Shutdown(0);
    return 0;
}
