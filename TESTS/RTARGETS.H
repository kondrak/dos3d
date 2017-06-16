#include "src/camera.h"
#include "src/math.h"
#include "src/timer.h"
#include "src/triangle.h"

// Multiple render targets test
void testRenderTargets()
{
    int i;
    unsigned long int now, last = 0;
    const unsigned short *keysPressed;
    gfx_Triangle triangle;
    gfx_Camera cam;
    mth_Matrix4 model;
    mth_Matrix4 modelViewProj;
    gfx_drawBuffer renderTargets[4];

    // create 4 draw buffers
    for(i = 0; i < 4; ++i)
        ALLOC_DRAWBUFFER(renderTargets[i], SCREEN_WIDTH/2, SCREEN_HEIGHT/2, DB_COLOR);

    for(i = 0; i < 4; ++i)
    {
        if(!DRAWBUFFER_VALID(renderTargets[i], DB_COLOR))
        {
            printf("Out of memory!");
            exit(1);
        }
    }

    tmr_start();

    mth_matIdentity(&model);
    mth_matIdentity(&modelViewProj);

    // setup camera
    VEC4(cam.position, 0, 0, 60);
    VEC4(cam.up, 0, 1, 0);
    VEC4(cam.right, 1, 0, 0);
    VEC4(cam.target, 0, 0, -1);

    triangle.color = rand() % 103 + 1;
    triangle.texture = NULL;
    VEC4(triangle.vertices[0].position, -20,  20, 0);
    VEC4(triangle.vertices[1].position,   0, -20, 0);
    VEC4(triangle.vertices[2].position,  20,  20, 0);

    keysPressed = kbd_getInput();
    mth_matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)renderTargets[0].width / (float)renderTargets[0].height, 0.1f, 500.f);

    while(!keysPressed[KEY_ESC])
    {
        float dt;
        now = tmr_getMs();
        dt = (float)(now - last);
        mth_matView(&cam.view, &cam.position, &cam.target, &cam.up);
        modelViewProj = mth_matMul(&cam.view, &cam.projection);
        modelViewProj = mth_matMul(&model, &modelViewProj);

        for(i = 0; i < 3; ++i)
        {
            mth_rotateVecAxisAngle(&triangle.vertices[i].position, 0.002f*dt, 1.f, 1.f, 1.f);
        }

        // blit triangle to 4 separate render targets
        for(i = 0; i < 4; ++i)
        {
            gfx_clrBufferColor(&renderTargets[i], i+1);
            gfx_drawTriangle(&triangle, &modelViewProj, &renderTargets[i]);
        }

        // blit render targets to screen
        gfx_blitBuffer(0, 0, &renderTargets[0], NULL);
        gfx_blitBuffer(SCREEN_WIDTH >> 1, 0, &renderTargets[1], NULL);
        gfx_blitBuffer(0, SCREEN_HEIGHT >> 1, &renderTargets[2], NULL);
        gfx_blitBuffer(SCREEN_WIDTH >> 1, SCREEN_HEIGHT >> 1, &renderTargets[3], NULL);

        keysPressed = kbd_getInput();
        last = now;
    }

    tmr_finish();

    for(i = 0; i < 4; ++i)
        FREE_DRAWBUFFER(renderTargets[i]);
}
