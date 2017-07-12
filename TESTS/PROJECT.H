#include "src/camera.h"
#include "src/graphics.h"
#include "src/timer.h"
#include "src/utils.h"

// helper function
void setupCube(mth_Vector4 *c);

// Perspective and orthographic projection test: render a rotating wireframe cube
void testProjection()
{
    uint32_t dt, now, last = tmr_getMs();
    float t = 0.f;
    int isPerspective = 1;
    mth_Vector4 cube[16];
    gfx_Camera cam;
    mth_Matrix4 model;
    mth_Matrix4 modelViewProj;
    gfx_drawBuffer buffer;

    ALLOC_DRAWBUFFER(buffer, SCREEN_WIDTH, SCREEN_HEIGHT, DB_COLOR);
    ASSERT(DRAWBUFFER_VALID(buffer, DB_COLOR), "Out of memory!\n");

    // setup camera
    VEC4(cam.position, 0, 0, 150);
    VEC4(cam.up, 0, 1, 0);
    VEC4(cam.right, 1, 0, 0);
    VEC4(cam.target, 0, 0, -1);

    // setup world coordinates for pseudo-cube
    setupCube(&cube);
    mth_matIdentity(&model);

    do
    {
        int i;
        mth_Vector4 screenCube[16];
        now = tmr_getMs();
        dt  = now - last;

        if(kbd_keyPressed(KEY_P))
            isPerspective = !isPerspective;

        if(isPerspective)
            mth_matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)buffer.width / buffer.height, 0.1f, 500.f);
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
            screenCube[i].x = (screenCube[i].x * buffer.width)  / (2.0f * screenCube[i].w) + (buffer.width  >> 1);
            screenCube[i].y = (screenCube[i].y * buffer.height) / (2.0f * screenCube[i].w) + (buffer.height >> 1);
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

        utl_printf(&buffer, 0, 1, 15, 0, "[P]rojection: %s", isPerspective ? "Perspective" : "Orthographic");
        gfx_updateScreen(&buffer);
        gfx_vSync();

        last = now;
    } while(!kbd_keyPressed(KEY_ESC));

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
