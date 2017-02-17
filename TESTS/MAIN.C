#include <ctype.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "src/input.h"
#include "tests/linedraw.h"
#include "tests/project.h"

static const int SCREEN_WIDTH  = 320;
static const int SCREEN_HEIGHT = 200;

// good bye
void Shutdown(int exitCode)
{
    setMode(0x03);
    exit(exitCode);
}

/* ***** */
void selectTest(char *nextTest)
{
    setMode(0x03);
    printf("Choose test (ESC to exit):\n");
    printf("1. Bresenham line drawing\n");
    printf("2. Perspective projection\n");
    printf("\nInput:\n");
    *nextTest = getch();
    setMode(0x13);
}

int main(int argc, char **argv)
{
    char nextTest;
    float invWidth, invHeight, aspectRatio, angle;
    int x, y, demoFinished = 0;
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

    selectTest(&nextTest);
  
    // calculate the view
    invWidth = 1.0f / SCREEN_WIDTH;
    invHeight = 1.0f / SCREEN_HEIGHT;
    aspectRatio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
    angle = tan(M_PI * 0.5 * fov / 180.);

    while (1)
    {
        if (!demoFinished)
        {
            for (x = 0; x < SCREEN_WIDTH; x++)
            {
                for (y = 0; y < SCREEN_HEIGHT; y++)
                {
                    drawPixel(x, y, 0);
                }
            }

            switch (nextTest)
            {
            case '1':
                testBresenham(160, 103, 90);
                break;
            case '2':
                testProjection();
                break;
            default:
                Shutdown(0);
            }
            
            demoFinished = 1;
        }
        else
        {
            getch();
            selectTest(&nextTest);
            demoFinished = 0;
        }
    }
}
