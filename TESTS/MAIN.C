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
    int x, y, demoFinished = 0;

    selectTest(&nextTest);
  
    while (1)
    {
        if (!demoFinished)
        {
            switch (nextTest)
            {
            case '1':
                testBresenham(160, 103, 90);
                break;
            case '2':
                testProjection(SCREEN_WIDTH, SCREEN_HEIGHT);
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
