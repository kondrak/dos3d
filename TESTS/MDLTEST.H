#include "src/camera.h"
#include "src/math.h"
#include "src/timer.h"
#include "src/triangle.h"
#include "src/utils.h"

#include "3rdparty/mdl/mdl.h"

// MDL rendering test
void testMdl()
{
    uint32_t dt, now, last = 0;
    const uint16_t *keysPressed;
    gfx_Camera cam;
    mth_Matrix4 modelViewProj;
    gfx_drawBuffer buffer;

    ALLOC_DRAWBUFFER(buffer, SCREEN_WIDTH, SCREEN_HEIGHT, DB_COLOR | DB_DEPTH);
    ASSERT(DRAWBUFFER_VALID(buffer, DB_COLOR | DB_DEPTH), "Out of memory!\n");

    // setup 1ms timer interrupt
    tmr_start();

    // setup camera
    VEC4(cam.position, 0, 0, 60);
    VEC4(cam.up, 0, 1, 0);
    VEC4(cam.right, 1, 0, 0);
    VEC4(cam.target, 0, 0, -1);

    mth_matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)buffer.width / buffer.height, 0.1f, 500.f);
    mth_matView(&cam.view, &cam.position, &cam.target, &cam.up);
    modelViewProj = mth_matMul(&cam.view, &cam.projection);

    do
    {
        now = tmr_getMs();
        dt  = now - last;
        keysPressed = kbd_getInput();

        // clear buffers and render model!
        gfx_clrBuffer(&buffer, DB_COLOR);
        gfx_updateScreen(&buffer);
        gfx_vSync();

        last = now;
    } while(!keysPressed[KEY_ESC]);

    tmr_finish();
    FREE_DRAWBUFFER(buffer);
}
