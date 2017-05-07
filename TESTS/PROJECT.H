#include "src/camera.h"
#include "src/graphics.h"
#include <stdlib.h>

// helper function
void setupCube(mth_Vector4 *c);

// Perspective projection test: render a rotating wireframe cube
void testPerspective()
{
    const unsigned short *keysPressed;
    float t = 0.f;
    mth_Vector4 cube[16];
    gfx_Camera cam;
    mth_Matrix4 model;
    mth_Matrix4 modelViewProj;
    unsigned char *buffer = (unsigned char *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT);

    if(!buffer)
    {
        printf("Out of memory!");
        exit(1);
    }

    mth_matIdentity(&model);
    mth_matIdentity(&modelViewProj);

    // setup camera
    cam.position.x = 0;
    cam.position.y = 0;
    cam.position.z = 150.f;
    cam.position.w = 1.f;

    cam.up.x = 0.f;
    cam.up.y = 1.f;
    cam.up.z = 0.f;
    cam.up.w = 1.f;

    cam.right.x = 1.f;
    cam.right.y = 0.f;
    cam.right.z = 0.f;
    cam.right.w = 1.f;

    cam.target.x = 0.f;
    cam.target.y = 0.f;
    cam.target.z = -1.f;
    cam.target.w = 1.f;

    // setup world coordinates for pseudo-cube
    setupCube(&cube);
    // translate entire cube a bit
    model.m[12] = -20.f;
    model.m[13] = -20.f;

    keysPressed = kbd_getInput();

    while(!keysPressed[KEY_ESC])
    {
        int i;
        // screen coordinates for the cube we will calculate here
        mth_Vector4 screenCube[16];
        mth_matView(&cam.view, &cam.position, &cam.target, &cam.up);
        mth_matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 5.f);
        modelViewProj = mth_matMul(&cam.view, &cam.projection);
        modelViewProj = mth_matMul(&model, &modelViewProj);

        t += 0.001f;
        for(i = 0; i < 16; i++)
        {
            // rotate cube on all axes, just for fun effect and zoom in/out for better perspective test
            mth_rotateVecAxisAngle(&cube[i], 0.001f, 1.f, 1.f, 1.f);
            model.m[14] = sin(t) * 80.f;
            
            // calculate screen coordinates for transformed square vertices
            screenCube[i] = mth_matMulVec(&modelViewProj, &cube[i]);

            // translate position to screen pixels
            screenCube[i].x = (screenCube[i].x * (float)SCREEN_WIDTH)  / (2.0f * screenCube[i].w) + (SCREEN_WIDTH >> 1);
            screenCube[i].y = (screenCube[i].y * (float)SCREEN_HEIGHT) / (2.0f * screenCube[i].w) + (SCREEN_HEIGHT >> 1);
        }

        gfx_clrBuffer(buffer);

        // render the cube walls
        for(i = 0; i < 4; i++)
        {
            gfx_drawLineVec(&screenCube[4*i + 0], &screenCube[4*i + 1], 3+i, buffer);
            gfx_drawLineVec(&screenCube[4*i + 1], &screenCube[4*i + 3], 3+i, buffer);
            gfx_drawLineVec(&screenCube[4*i + 0], &screenCube[4*i + 2], 3+i, buffer);
            gfx_drawLineVec(&screenCube[4*i + 2], &screenCube[4*i + 3], 3+i, buffer);
        }

        gfx_updateScreen(buffer);

        keysPressed = kbd_getInput();
    }
    
    free(buffer);
}


void setupCube(mth_Vector4 *c)
{
    int i;
    for(i = 0; i < 4; i++)
    {
        c[i].x = 40*(i%2);
        c[i].y = 40*(i > 1 ? 1 : 0);
        c[i].z = 0.f;
        c[i].w = 1.f;
    }
    
    for(i = 4; i < 8; i++)
    {
        c[i].x = 40;
        c[i].y = 40*(i > 5 ? 1 : 0);
        c[i].z = 40*(i%2);
        c[i].w = 1.f;
    }
    
    for(i = 8; i < 12; i++)
    {
        c[i].x = 40*(i%2);
        c[i].y = 40*(i > 9 ? 1 : 0);
        c[i].z = 40;
        c[i].w = 1.f;
    }
    
    for(i = 12; i < 16; i++)
    {
        c[i].x = 0;
        c[i].y = 40*(i > 13 ? 1 : 0);
        c[i].z = 40*(i%2);
        c[i].w = 1.f;
    }
}
