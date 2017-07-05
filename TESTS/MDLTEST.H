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
    int i;
    uint32_t t = 0;
    const uint16_t *keysPressed;
    gfx_Camera cam;
    mth_Matrix4 modelViewProj;
    mth_Matrix4 modelMatrix;
    mdl_model_t mdl;
    gfx_drawBuffer buffer;

    ALLOC_DRAWBUFFER(buffer, SCREEN_WIDTH, SCREEN_HEIGHT, DB_COLOR | DB_DEPTH);
    ASSERT(DRAWBUFFER_VALID(buffer, DB_COLOR | DB_DEPTH), "Out of memory!\n");
    ReadMDLModel("images/shambler.mdl", &mdl);
    //buffer.drawOpts.drawMode = DM_WIREFRAME;
    buffer.drawOpts.depthFunc = DF_LESS;
    buffer.drawOpts.cullMode = FC_BACK;

    // setup 1ms timer interrupt
    tmr_start();

    // setup camera
    VEC4(cam.position, 120, 0, 30);
    VEC4(cam.up, 0, 0, -1);
    VEC4(cam.right, 0, 1, 0);
    VEC4(cam.target, -1, 0, 0);

    mth_matIdentity(&modelMatrix);
    mth_matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)buffer.width / buffer.height, 0.1f, 500.f);
    mth_matView(&cam.view, &cam.position, &cam.target, &cam.up);
    modelViewProj = mth_matMul(&cam.view, &cam.projection);

    gfx_setPalette8(mdl.skinTextures[0].palette);

    do
    {
        now = tmr_getMs();
        dt  = now - last;
        keysPressed = kbd_getInput();
        t += dt;

        //modelMatrix.m[14] = sin(t) * 80.f;
        //modelViewProj = mth_matMul(&modelMatrix, &modelViewProj);


        // clear buffers and render model!
        gfx_clrBufferColor(&buffer, 3);
        gfx_clrBuffer(&buffer, DB_DEPTH);
        RenderFrame(i, &mdl, &modelViewProj, &buffer);
        gfx_updateScreen(&buffer);
        gfx_vSync();

        last = now;

        if(t > 75)
        {
            i++;
            t = 0;
        }

        if(i > mdl.header.num_frames)
            i = 0;
    } while(!keysPressed[KEY_ESC]);

    tmr_finish();
    FreeModel(&mdl);
    FREE_DRAWBUFFER(buffer);
}
