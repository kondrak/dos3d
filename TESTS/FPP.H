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
    cam.position.x = 0;
    cam.position.y = 0;
    cam.position.z = 40.f;
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
    
    triangle.color = 3;
    triangle.texture = NULL;
    triangle.vertices[0].position.x = -10;
    triangle.vertices[0].position.y = 10;
    triangle.vertices[0].position.z = 0;
    triangle.vertices[0].position.w = 1.f;

    triangle.vertices[1].position.x = 10;
    triangle.vertices[1].position.y = 0;
    triangle.vertices[1].position.z = 0;
    triangle.vertices[1].position.w = 1.f;

    triangle.vertices[2].position.x = -10;
    triangle.vertices[2].position.y = 0;
    triangle.vertices[2].position.z = 0;
    triangle.vertices[2].position.w = 1.f;

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
