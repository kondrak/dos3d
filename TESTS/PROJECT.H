#include "src/camera.h"
#include "src/graphics.h"
#include "src/timer.h"
#include "src/utils.h"

// helper function
void setupCube(mth_Vector4 *c);

// Perspective projection test: render a rotating wireframe cube
void testPerspective()
{
    const unsigned short *keysPressed;
    unsigned long int now, last = 0;
    float t = 0.f;
    int projectionFlipped = 0;
    int isPerspective = 1;
    mth_Vector4 cube[16];
    gfx_Camera cam;
    mth_Matrix4 model;
    mth_Matrix4 modelViewProj;
    gfx_drawBuffer buffer;

    ALLOC_DRAWBUFFER(buffer, SCREEN_WIDTH, SCREEN_HEIGHT, DB_COLOR);

    if(!DRAWBUFFER_VALID(buffer, DB_COLOR))
    {
        printf("Out of memory!");
        exit(1);
    }

    tmr_start();

    mth_matIdentity(&model);
    mth_matIdentity(&modelViewProj);

    // setup camera
    VEC4(cam.position, 0, 0, 150);
    VEC4(cam.up, 0, 1, 0);
    VEC4(cam.right, 1, 0, 0);
    VEC4(cam.target, 0, 0, -1);

    // setup world coordinates for pseudo-cube
    setupCube(&cube);

    do
    {
        int i;
        float dt;
        // screen coordinates for the cube we will calculate here
        mth_Vector4 screenCube[16];
        now = tmr_getMs();
        dt = (float)(now - last);
        keysPressed = kbd_getInput();

        if(keysPressed[KEY_P] && !projectionFlipped)
        {
            isPerspective = !isPerspective;
            projectionFlipped = 1;
        }
        else if(!keysPressed[KEY_P]) projectionFlipped = 0;

        if(isPerspective)
            mth_matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)buffer.width / (float)buffer.height, 0.1f, 500.f);
        else
            mth_matOrtho(&cam.projection, -160, 160, 100, -100, 0.1f, 500.f);

        mth_matView(&cam.view, &cam.position, &cam.target, &cam.up);
        modelViewProj = mth_matMul(&cam.view, &cam.projection);
        modelViewProj = mth_matMul(&model, &modelViewProj);

        t += 0.0015f*dt;
        for(i = 0; i < 16; ++i)
        {
            // rotate cube on x axis and zoom in/out for better perspective test
            mth_rotateVecAxisAngle(&cube[i], 0.0015f*dt, 1.f, 0.f, 0.f);
            model.m[14] = sin(t) * 80.f;
            
            // calculate screen coordinates for transformed square vertices
            screenCube[i] = mth_matMulVec(&modelViewProj, &cube[i]);

            // translate position to screen pixels
            screenCube[i].x = (screenCube[i].x * (float)buffer.width)  / (2.0f * screenCube[i].w) + (buffer.width >> 1);
            screenCube[i].y = (screenCube[i].y * (float)buffer.height) / (2.0f * screenCube[i].w) + (buffer.height >> 1);
        }

        gfx_clrBuffer(&buffer, DB_COLOR);

        // render the cube walls
        for(i = 0; i < 4; ++i)
        {
            gfx_drawLineVec(&screenCube[4*i + 0], &screenCube[4*i + 1], 3+i, &buffer);
            gfx_drawLineVec(&screenCube[4*i + 1], &screenCube[4*i + 3], 3+i, &buffer);
            gfx_drawLineVec(&screenCube[4*i + 0], &screenCube[4*i + 2], 3+i, &buffer);
            gfx_drawLineVec(&screenCube[4*i + 2], &screenCube[4*i + 3], 3+i, &buffer);
        }

        utl_printf(isPerspective ? "[P]rojection: Perspective" : "[P]rojection: Orthographic", 0, 1, 15, 0, &buffer);
        gfx_updateScreen(&buffer);
        gfx_vSync();

        last = now;
    } while(!keysPressed[KEY_ESC]);

    tmr_finish();
    FREE_DRAWBUFFER(buffer);
}

/* ***** */
void setupCube(mth_Vector4 *c)
{
    int i;
    for(i = 0; i < 4; ++i)
    {
        c[i].x = 20*(i%2 ? 1 : -1);
        c[i].y = 20*(i > 1 ? 1 : -1);
        c[i].z = -20.f;
        c[i].w = 1.f;
    }
    
    for(i = 4; i < 8; ++i)
    {
        c[i].x = 20;
        c[i].y = 20*(i > 5 ? 1 : -1);
        c[i].z = 20*(i%2 ? 1 : -1);
        c[i].w = 1.f;
    }
    
    for(i = 8; i < 12; ++i)
    {
        c[i].x = 20*(i%2 ? 1 : -1);
        c[i].y = 20*(i > 9 ? 1 : -1);
        c[i].z = 20;
        c[i].w = 1.f;
    }
    
    for(i = 12; i < 16; ++i)
    {
        c[i].x = -20;
        c[i].y = 20*(i > 13 ? 1 : -1);
        c[i].z = 20*(i%2 ? 1 : -1);
        c[i].w = 1.f;
    }
}
