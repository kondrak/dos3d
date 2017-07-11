#include "src/camera.h"
#include "src/timer.h"
#include "src/triangle.h"

// First person WASD camera (warning: a bit faulty with z-clipping until proper clipping is implemented!)
void testFirstPerson()
{
    uint32_t dt, now, last = tmr_getMs();
    mth_Vector4 ts, rs;
    gfx_Camera cam;
    gfx_Triangle triangle;
    mth_Matrix4 viewProj;
    gfx_drawBuffer buffer;

    ALLOC_DRAWBUFFER(buffer, SCREEN_WIDTH, SCREEN_HEIGHT, DB_COLOR | DB_DEPTH);
    ASSERT(DRAWBUFFER_VALID(buffer, DB_COLOR | DB_DEPTH), "Out of memory!\n");
    buffer.drawOpts.depthFunc = DF_LESS;

    // setup camera
    VEC4(cam.position, 0, 0, 40);
    VEC4(cam.up, 0, 1, 0);
    VEC4(cam.right, 1, 0, 0);
    VEC4(cam.target, 0, 0, -1);
    
    triangle.color = 3;
    triangle.texture = NULL;
    VEC4(triangle.vertices[0].position, -10, 10, 0);
    VEC4(triangle.vertices[1].position,  10,  0, 0);
    VEC4(triangle.vertices[2].position, -10,  0, 0);

    mth_matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)buffer.width / buffer.height, 1.0f, 500.f);

    do
    {
        now = tmr_getMs();
        dt  = now - last;

        mth_matView(&cam.view, &cam.position, &cam.target, &cam.up);
        viewProj = mth_matMul(&cam.view, &cam.projection);

        gfx_clrBuffer(&buffer, DB_COLOR | DB_DEPTH);
        gfx_drawTriangle(&triangle, &viewProj, &buffer);
        gfx_updateScreen(&buffer);

        ts = mth_vecScale(&cam.target, 0.05f*dt);
        rs = mth_vecScale(&cam.right, 0.05f*dt);

        if(kbd_keyDown(KEY_W))
            cam.position = mth_vecAdd(&cam.position, &ts);

        if(kbd_keyDown(KEY_S))
            cam.position = mth_vecSub(&cam.position, &ts);

        if(kbd_keyDown(KEY_A))
            cam.position = mth_vecSub(&cam.position, &rs);

        if(kbd_keyDown(KEY_D))
            cam.position = mth_vecAdd(&cam.position, &rs);

        if(kbd_keyDown(KEY_LEFT))
        {
            mth_rotateVecAxisAngle(&cam.target, 0.002f*dt, cam.up.x, cam.up.y, cam.up.z);
            cam.right = mth_crossProduct(&cam.target, &cam.up);
        }

        if(kbd_keyDown(KEY_RIGHT))
        {
            mth_rotateVecAxisAngle(&cam.target, -0.002f*dt, cam.up.x, cam.up.y, cam.up.z);
            cam.right = mth_crossProduct(&cam.target, &cam.up);
        }

        if(kbd_keyDown(KEY_R))
        {
            mth_rotateVecAxisAngle(&cam.target, -0.002f*dt, cam.right.x, cam.right.y, cam.right.z);
            cam.up = mth_crossProduct(&cam.right, &cam.target);
        }

        if(kbd_keyDown(KEY_F))
        {
            mth_rotateVecAxisAngle(&cam.target, 0.002f*dt, cam.right.x, cam.right.y, cam.right.z);
            cam.up = mth_crossProduct(&cam.right, &cam.target);
        }

        gfx_vSync();

        last = now;
    } while(!kbd_keyPressed(KEY_ESC));

    FREE_DRAWBUFFER(buffer);
}
