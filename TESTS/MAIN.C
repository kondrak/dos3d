#include <ctype.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "src/graphics.h"
#include "src/input.h"
#include "tests/3dscene.h"
#include "tests/cube.h"
#include "tests/fpp.h"
#include "tests/linedraw.h"
#include "tests/mdltest.h"
#include "tests/project.h"
#include "tests/rtargets.h"
#include "tests/texmap.h"
#include "tests/tris.h"

void printMenu()
{
    gfx_setMode(0x03);
    printf("*** DOS3D software renderer test suite ***\n");
    printf("1. Bresenham line drawing\n");
    printf("2. Projection test\n");
    printf("3. Triangle rendering\n");
    printf("4. Render targets\n");
    printf("5. Texture mapping\n");
    printf("6. Rotating cube\n");
    printf("7. Test 3D scene\n");
    printf("8. MDL rendering\n");
    printf("9. First person WASD camera\n");
    printf("\nq. Exit!\n");
    printf("\nInput:\n");
}

// main test program
int main(int argc, char **argv)
{
    int demoFinished = 0;
    printMenu();

    // setup 1ms timer interrupt
    tmr_start();
    // setup keyboard interrupt
    kbd_start();

    while(1)
    {
        if (!demoFinished)
        {
            if(kbd_keyPressed(KEY_1))
            {
                gfx_setMode(0x13);
                testBresenham(160, 103, 90);
                demoFinished = 1;
            }
            if(kbd_keyPressed(KEY_2))
            {
                gfx_setMode(0x13);
                testProjection();
                demoFinished = 1;
            }
            if(kbd_keyPressed(KEY_3))
            {
                gfx_setMode(0x13);
                testTriangles();
                demoFinished = 1;
            }
            if(kbd_keyPressed(KEY_4))
            {
                gfx_setMode(0x13);
                testRenderTargets();
                demoFinished = 1;
            }
            if(kbd_keyPressed(KEY_5))
            {
                gfx_setMode(0x13);
                testTextureMapping();
                demoFinished = 1;
            }
            if(kbd_keyPressed(KEY_6))
            {
                gfx_setMode(0x13);
                testRotatingCube();
                demoFinished = 1;
            }
            if(kbd_keyPressed(KEY_7))
            {
                gfx_setMode(0x13);
                test3DScene();
                demoFinished = 1;
            }
            if(kbd_keyPressed(KEY_8))
            {
                gfx_setMode(0x13);
                testMdl();
                demoFinished = 1;
            }
            if(kbd_keyPressed(KEY_9))
            {
                gfx_setMode(0x13);
                testFirstPerson();
                demoFinished = 1;
            }
            // exit
            if(kbd_keyPressed(KEY_Q))
                break;
        }
        else
        {
            printMenu();
            demoFinished = 0;
        }
    }

    gfx_setMode(0x03);
    // reset original keyboard interrupt
    kbd_finish();
    // reset original timer interrupt
    tmr_finish();

    return 0;
}
