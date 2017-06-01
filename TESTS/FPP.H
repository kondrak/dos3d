#include "src/camera.h"
#include "src/graphics.h"
#include <stdlib.h>

// First person WASD camera
void testFirstPerson()
{
    const unsigned short *keysPressed;
    unsigned int i;
    mth_Vector4 ts, rs;
    mth_Vector4 square[4];
    gfx_Camera cam;
    mth_Matrix4 viewProj;
    unsigned char *buffer = (unsigned char *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT);

    // setup camera
    cam.position.x = 0;
    cam.position.y = 0;
    cam.position.z = 1.f;
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
    
    if(!buffer)
    {
        printf("Out of memory!");
        exit(1);
    }
    
    keysPressed = kbd_getInput();

    while(!keysPressed[KEY_ESC])
    {
        for(i = 0; i < 4; i++)
        {
            square[i].x = 0 + 50*(i%2);
            square[i].y = 0 + 50*(i > 1 ? 1 : 0);
            square[i].z = -80.f;
            square[i].w = 1.f;
        }

        mth_matView(&cam.view, &cam.position, &cam.target, &cam.up);
        mth_matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 500.f);
        viewProj = mth_matMul(&cam.view, &cam.projection);

        for(i = 0; i < 4; i++)
        {
            square[i] = mth_matMulVec(&viewProj, &square[i]);

            // translate position to screen pixels
            square[i].x = (square[i].x * (float)SCREEN_WIDTH)  / (2.0f * square[i].w) + (SCREEN_WIDTH >> 1);
            square[i].y = (square[i].y * (float)SCREEN_HEIGHT) / (2.0f * square[i].w) + (SCREEN_HEIGHT >> 1);
        }

        gfx_clrBuffer(buffer);

        gfx_drawLineVec(&square[0], &square[1], 3, buffer);
        gfx_drawLineVec(&square[1], &square[3], 3, buffer);
        gfx_drawLineVec(&square[0], &square[2], 3, buffer);
        gfx_drawLineVec(&square[2], &square[3], 3, buffer);

        gfx_updateScreen(buffer);

        keysPressed = kbd_getInput();

        ts = mth_vecScale(&cam.target, 0.1f);
        rs = mth_vecScale(&cam.right, 0.1f);

        if(keysPressed[KEY_W])
            cam.position = mth_vecAdd(&cam.position, &ts);

        if(keysPressed[KEY_S])
            cam.position = mth_vecSub(&cam.position, &ts);

        if(keysPressed[KEY_A])
            cam.position = mth_vecSub(&cam.position, &rs);

        if(keysPressed[KEY_D])
            cam.position = mth_vecAdd(&cam.position, &rs);

        if(keysPressed[KEY_LEFT])
        {
            mth_rotateVecAxisAngle(&cam.target, 0.001f, cam.up.x, cam.up.y, cam.up.z);
            cam.right = mth_crossProduct(&cam.target, &cam.up);
        }

        if(keysPressed[KEY_RIGHT])
        {
            mth_rotateVecAxisAngle(&cam.target, -0.001f, cam.up.x, cam.up.y, cam.up.z);
            cam.right = mth_crossProduct(&cam.target, &cam.up);
        }

        if(keysPressed[KEY_PGUP])
        {
            mth_rotateVecAxisAngle(&cam.target, -0.001f, cam.right.x, cam.right.y, cam.right.z);
            cam.right = mth_crossProduct(&cam.target, &cam.up);
        }

        if(keysPressed[KEY_PGDN])
        {
            mth_rotateVecAxisAngle(&cam.target, 0.001f, cam.right.x, cam.right.y, cam.right.z);
            cam.up = mth_crossProduct(&cam.right, &cam.target);
        }
    }

    free(buffer);
}
