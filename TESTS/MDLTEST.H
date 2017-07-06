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

enum ShamblerAnimations
{
    IDLE = 0,
    WALK,
    RUN,
    ATTACK1,
    ATTACK2,
    ATTACK3,
    THUNDERBOLT,
    HIT,
    DEAD,
    NUM_ANIMS
};

// MDL rendering test
void testMdl()
{
    const char *animNames[NUM_ANIMS+1] = { "Idle", "Walk", "Run", "Attack1", 
                                           "Attack2", "Attack3", "Thunderbolt", 
                                           "Hit", "Dead", "Cycle" };
    const int animFrames[NUM_ANIMS][2] = {   {0, 16}, {17, 29}, {30, 35}, 
                                            {36, 47}, {48, 58}, {59, 65},
                                            {66, 74}, {75, 82}, {83, 94} };
    const uint16_t *keysPressed;
    uint32_t dt, now, last = 0;
    int freezeFlipped = 0, drawModeFlipped = 0, animFlipped = 0;
    int frameNum = 0, startFrame = 0, endFrame;
    int currAnim = NUM_ANIMS;
    int drawMode = HALF_WIREFRAME;
    int freezeFrame = 0;
    int frameTimeSkew = 25;
    int mdlAnimated = 0;
    float frameLerp = 0.f;
    float t = 0.f;
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

    mdl_load("images/shambler.mdl", &mdl);
    fullBuffer.drawOpts.depthFunc = DF_LESS;
    fullBuffer.drawOpts.cullMode  = FC_BACK;
    halfBuffer.drawOpts.depthFunc = DF_LESS;
    halfBuffer.drawOpts.cullMode  = FC_BACK;
    wireFrameBuffer.drawOpts.drawMode = DM_WIREFRAME;
    wireFrameBuffer.drawOpts.cullMode = FC_BACK;
    endFrame = mdl.header.num_frames;
    mdlAnimated = mdl.header.num_frames > 1;
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
        t += 0.0005f * dt;

        // animate model frames
        if(mdlAnimated)
        {
            frameLerp += 0.0005f * dt * frameTimeSkew;
            mdl_animate(startFrame, endFrame-1, &frameNum, &frameLerp);
        }

        // circular rotation around the model
        modelMatrix.m[12] = ROUND(120 * sin(t));
        modelMatrix.m[13] = ROUND(120 * cos(t));
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

        if(keysPressed[KEY_A] && !animFlipped)
        {
            if(currAnim == NUM_ANIMS)
                currAnim = -1;

            currAnim++;

            if(currAnim != NUM_ANIMS)
            {
                startFrame = animFrames[currAnim][0];
                endFrame   = animFrames[currAnim][1];
            }
            else
            {
                startFrame = 0;
                endFrame   = mdl.header.num_frames;
            }

            frameNum = startFrame;
            frameLerp = 0.f;
            animFlipped = 1;
        }
        else if(!keysPressed[KEY_A]) animFlipped = 0;

        if(keysPressed[KEY_F] && !freezeFlipped)
        {
            freezeFrame = !freezeFrame;
            freezeFlipped = 1;
        }
        else if(!keysPressed[KEY_F]) freezeFlipped = 0;

        if(keysPressed[KEY_PLUS])
        {
            frameTimeSkew += frameTimeSkew >= 100 ? 0 : 1;
        }

        if(keysPressed[KEY_MINUS])
        {
            frameTimeSkew -= frameTimeSkew <= 1 ? 0 : 1;
        }

        // clear buffers and render!
        if(drawMode == HALF_WIREFRAME)
        {
            gfx_clrBufferColor(&wireFrameBuffer, 2);
            gfx_clrBufferColor(&halfBuffer, 3);
            gfx_clrBuffer(&halfBuffer, DB_DEPTH);

            if(mdlAnimated)
            {
                mdl_renderFrameLerp(frameNum, frameLerp, &mdl, &modelViewProj, &halfBuffer);
                mdl_renderFrameLerp(frameNum, frameLerp, &mdl, &modelViewProj, &wireFrameBuffer);
            }
            else
            {
                mdl_renderFrame(frameNum, &mdl, &modelViewProj, &halfBuffer);
                mdl_renderFrame(frameNum, &mdl, &modelViewProj, &wireFrameBuffer);
            }

            gfx_blitBuffer(0, 0, &halfBuffer, &fullBuffer);
            gfx_blitBuffer(SCREEN_WIDTH >> 1, 0, &wireFrameBuffer, &fullBuffer);
        }
        else
        {
            gfx_clrBufferColor(&fullBuffer, 3);
            gfx_clrBuffer(&fullBuffer, DB_DEPTH);

            if(mdlAnimated)
                mdl_renderFrameLerp(frameNum, frameLerp, &mdl, &modelViewProj, &fullBuffer);
            else
                mdl_renderFrame(frameNum, &mdl, &modelViewProj, &fullBuffer);
        }

        utl_printf(&fullBuffer, 0, 1, 15, 0, "[F]rame: %03d/%03d @ %dx [+/-]", frameNum+1, mdl.header.num_frames, frameTimeSkew);
        utl_printf(&fullBuffer, 0, 10, 15, 0, "[A]nim : %s", animNames[currAnim]);

        gfx_updateScreen(&fullBuffer);
        gfx_vSync();

        last = now;
    } while(!keysPressed[KEY_ESC]);

    tmr_finish();
    mdl_free(&mdl);
    FREE_DRAWBUFFER(fullBuffer);
    FREE_DRAWBUFFER(halfBuffer);
    FREE_DRAWBUFFER(wireFrameBuffer);
}
