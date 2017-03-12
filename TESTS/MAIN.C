#include <ctype.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "src/graphics.h"
#include "src/input.h"
#include "tests/fpp.h"
#include "tests/linedraw.h"
#include "tests/project.h"

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
    printf("3. First person WASD camera\n");
    printf("\nInput:\n");
    *nextTest = getch();
}

int main(int argc, char **argv)
{
    char nextTest;
    int demoFinished = 0;

    selectTest(&nextTest);
  
    while (1)
    {
        if (!demoFinished)
        {
            switch (nextTest)
            {
            case '1':
                setMode(0x13);
                testBresenham(160, 103, 90);
                break;
            case '2':
                setMode(0x13);
                testPerspective();
                break;
            case '3':
                setMode(0x13);
                testFirstPerson();
                break;
            case 27:
                Shutdown(0);
            default:
                break;
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
