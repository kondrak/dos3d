#include "src/input.h"
#include "src/dt_math.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static const int SCREEN_WIDTH  = 320;
static const int SCREEN_HEIGHT = 200;

// pointer to VGA memory
unsigned char *VGA = (unsigned char *)0xA0000000L;

// graphics mode setter
void setMode(unsigned char mode)
{
    _asm {
	    mov ah, 0x00
	    mov al, mode
	    int 10h
    }
}

// good bye
void Shutdown(int exitCode)
{
    setMode(0x03);
    exit(exitCode);
}

/* ***** */

int main(int argc, char **argv)
{
    float invWidth, invHeight, aspectRatio, angle;
    int x, y, renderFinished = 0;
    int fov = 45;
    unsigned short *keysPressed = translateInput();

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
                    // draw the pixel
                    VGA[(y << 8) + (y << 6) + x] = 3;
                }
            }
            renderFinished = 1;
        }
        else
        {
            keysPressed = translateInput();
        }
    }

    Shutdown(0);
    return 0;
}
