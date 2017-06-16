#include "src/camera.h"
#include "src/timer.h"
#include "src/triangle.h"

// First person WASD camera (warning: a bit faulty with z-clipping until proper clipping is implemented!)
void testFirstPerson()
{
    unsigned long int now, last = 0;
    const unsigned short *keysPressed;
    mth_Vector4 ts, rs;
    gfx_Camera cam;
    gfx_Triangle triangle;
    mth_Matrix4 viewProj;
    gfx_drawBuffer buffer;

    ALLOC_DRAWBUFFER(buffer, SCREEN_WIDTH, SCREEN_HEIGHT, DB_COLOR | DB_DEPTH);
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

    if(!DRAWBUFFER_VALID(buffer, DB_COLOR | DB_DEPTH))
    {
        printf("Out of memory!");
        exit(1);
    }

    tmr_start();
    keysPressed = kbd_getInput();
    mth_matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)buffer.width / (float)buffer.height, 1.0f, 500.f);

    while(!keysPressed[KEY_ESC])
    {
        float dt;
        now = tmr_getMs();
        dt = (float)(now - last);
        mth_matView(&cam.view, &cam.position, &cam.target, &cam.up);
        viewProj = mth_matMul(&cam.view, &cam.projection);

        gfx_clrBuffer(&buffer, DB_COLOR | DB_DEPTH);
        gfx_drawTriangle(&triangle, &viewProj, &buffer);
        gfx_updateScreen(&buffer);

        keysPressed = kbd_getInput();

        ts = mth_vecScale(&cam.target, 0.05f*dt);
        rs = mth_vecScale(&cam.right, 0.05f*dt);

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
            mth_rotateVecAxisAngle(&cam.target, 0.002f*dt, cam.up.x, cam.up.y, cam.up.z);
            cam.right = mth_crossProduct(&cam.target, &cam.up);
        }

        if(keysPressed[KEY_RIGHT])
        {
            mth_rotateVecAxisAngle(&cam.target, -0.002f*dt, cam.up.x, cam.up.y, cam.up.z);
            cam.right = mth_crossProduct(&cam.target, &cam.up);
        }

        if(keysPressed[KEY_R])
        {
            mth_rotateVecAxisAngle(&cam.target, -0.002f*dt, cam.right.x, cam.right.y, cam.right.z);
            cam.up = mth_crossProduct(&cam.right, &cam.target);
        }

        if(keysPressed[KEY_F])
        {
            mth_rotateVecAxisAngle(&cam.target, 0.002f*dt, cam.right.x, cam.right.y, cam.right.z);
            cam.up = mth_crossProduct(&cam.right, &cam.target);
        }
        last = now;
    }

    tmr_finish();
    FREE_DRAWBUFFER(buffer);
}
