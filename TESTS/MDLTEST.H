#include "src/camera.h"
#include "src/math.h"
#include "src/timer.h"
#include "src/triangle.h"
#include "src/utils.h"

#include "3rdparty/mdl/mdl.h"

enum MDLDrawModeType
{
    HALF_WIREFRAME, // textured screen on one half, wireframe on second half
    FULLSCREEN,     // fullscreen model
    MDLDRAWMODE_CNT
};

// MDL rendering test
void testMdl()
{
    uint32_t dt, now, last = 0;
    int i, drawModeFlipped = 0;
    int drawMode = HALF_WIREFRAME;
    uint32_t frameTimer = 0;
    float rotTimer = 0.f;
    const uint16_t *keysPressed;
    gfx_Camera cam;
    mth_Matrix4 modelViewProj;
    mth_Matrix4 modelMatrix;
    mdl_model_t mdl;
    gfx_drawBuffer fullBuffer, halfBuffer;
    gfx_drawBuffer wireFrameBuffer;

    ALLOC_DRAWBUFFER(fullBuffer, SCREEN_WIDTH, SCREEN_HEIGHT, DB_COLOR | DB_DEPTH);
    ASSERT(DRAWBUFFER_VALID(fullBuffer, DB_COLOR | DB_DEPTH), "Out of memory!\n");

    ALLOC_DRAWBUFFER(halfBuffer, SCREEN_WIDTH >> 1, SCREEN_HEIGHT, DB_COLOR | DB_DEPTH);
    ASSERT(DRAWBUFFER_VALID(halfBuffer, DB_COLOR | DB_DEPTH), "Out of memory!\n");

    ALLOC_DRAWBUFFER(wireFrameBuffer, SCREEN_WIDTH >> 1, SCREEN_HEIGHT, DB_COLOR);
    ASSERT(DRAWBUFFER_VALID(wireFrameBuffer, DB_COLOR), "Out of memory!\n");

    ReadMDLModel("images/shambler.mdl", &mdl);
    fullBuffer.drawOpts.depthFunc = DF_LESS;
    fullBuffer.drawOpts.cullMode  = FC_BACK;
    halfBuffer.drawOpts.depthFunc = DF_LESS;
    halfBuffer.drawOpts.cullMode  = FC_BACK;
    wireFrameBuffer.drawOpts.drawMode = DM_WIREFRAME;
    wireFrameBuffer.drawOpts.cullMode = FC_BACK;

    // setup 1ms timer interrupt
    tmr_start();

    // setup camera
    VEC4(cam.position, 0, 0, 30);
    VEC4(cam.up, 0, 0, -1);
    VEC4(cam.right, 0, 1, 0);
    VEC4(cam.target, -1, 0, 0);

    mth_matIdentity(&modelMatrix);
    mth_matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)halfBuffer.width / halfBuffer.height, 0.1f, 500.f);

    gfx_setPalette8(mdl.skinTextures[0].palette);

    do
    {
        now = tmr_getMs();
        dt  = now - last;
        keysPressed = kbd_getInput();
        frameTimer += dt;
        rotTimer   += 0.0005f * dt;

        // circular rotation around the model
        modelMatrix.m[12] = ROUND(120 * sin(rotTimer));
        modelMatrix.m[13] = ROUND(120 * cos(rotTimer));
        cam.target.x = modelMatrix.m[12];
        cam.target.y = modelMatrix.m[13];
        mth_matView(&cam.view, &cam.position, &cam.target, &cam.up);
        modelViewProj = mth_matMul(&cam.view, &cam.projection);
        modelViewProj = mth_matMul(&modelMatrix, &modelViewProj);

        if(keysPressed[KEY_SPACE] && !drawModeFlipped)
        {
            drawMode++;
            if(drawMode == MDLDRAWMODE_CNT)
                drawMode = HALF_WIREFRAME;

            if(drawMode == HALF_WIREFRAME)
                mth_matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)halfBuffer.width / halfBuffer.height, 0.1f, 500.f);
            else
                mth_matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)fullBuffer.width / fullBuffer.height, 0.1f, 500.f);

            drawModeFlipped = 1;
        }
        else if(!keysPressed[KEY_SPACE]) drawModeFlipped = 0;

        // clear buffers and render!
        if(drawMode == HALF_WIREFRAME)
        {
            gfx_clrBufferColor(&wireFrameBuffer, 2);
            gfx_clrBufferColor(&halfBuffer, 3);
            gfx_clrBuffer(&halfBuffer, DB_DEPTH);
            RenderFrame(i, &mdl, &modelViewProj, &halfBuffer);
            RenderFrame(i, &mdl, &modelViewProj, &wireFrameBuffer);

            gfx_blitBuffer(0, 0, &halfBuffer, NULL);
            gfx_blitBuffer(SCREEN_WIDTH >> 1, 0, &wireFrameBuffer, NULL);
        }
        else
        {
            gfx_clrBufferColor(&fullBuffer, 3);
            gfx_clrBuffer(&fullBuffer, DB_DEPTH);
            RenderFrame(i, &mdl, &modelViewProj, &fullBuffer);

            gfx_updateScreen(&fullBuffer);
        }

        gfx_vSync();
        last = now;

        if(frameTimer > 75)
        {
            i++;
            frameTimer = 0;
        }

        if(i > mdl.header.num_frames)
            i = 0;
    } while(!keysPressed[KEY_ESC]);

    tmr_finish();
    FreeModel(&mdl);
    FREE_DRAWBUFFER(fullBuffer);
    FREE_DRAWBUFFER(halfBuffer);
    FREE_DRAWBUFFER(wireFrameBuffer);
}
