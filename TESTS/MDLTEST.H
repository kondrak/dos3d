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
    ALL_ANIMS // cycling all animations in order
};

// MDL rendering test
void testMdl()
{
    const char *animNames[ALL_ANIMS+1] = { "Idle", "Walk", "Run", "Attack1", 
                                           "Attack2", "Attack3", "Thunderbolt", 
                                           "Hit", "Dead", "Cycle" };
    const int animFrames[ALL_ANIMS][2] = {   {0, 16}, {17, 29}, {30, 35}, 
                                            {36, 47}, {48, 58}, {59, 65},
                                            {66, 74}, {75, 82}, {83, 94} };
    const int distFromModel = 120;
    uint32_t dt, now, last = tmr_getMs();
    int rotDir, rotating = 1;
    int frameNum = 0, startFrame = 0, endFrame;
    int currAnim = ALL_ANIMS;
    int drawMode = HALF_WIREFRAME;
    int freezeFrame = 0;
    int frameTimeSkew = 25;
    int mdlAnimated = 0;
    float frameLerp = 0.f;
    float distScale = 1.f;
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

    endFrame    = mdl.header.num_frames;
    mdlAnimated = mdl.header.num_frames > 1;

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

        // animate model frames
        if(mdlAnimated)
        {
            frameLerp += 0.0005f * dt * frameTimeSkew;
            mdl_animate(startFrame, endFrame-1, &frameNum, &frameLerp);
        }

        // circular rotation around the model
        rotDir     = kbd_keyDown(KEY_RIGHT) ? -4 : kbd_keyDown(KEY_LEFT) ? 4 : 0;
        distScale += kbd_keyDown(KEY_UP) ? -0.1f : kbd_keyDown(KEY_DOWN) ? 0.1f : 0.f;
        t += 0.0005f * dt * (rotating + !rotating * rotDir);
        modelMatrix.m[12] = distScale * distFromModel * sin(t);
        modelMatrix.m[13] = distScale * distFromModel * cos(t);
        cam.target.x = modelMatrix.m[12];
        cam.target.y = modelMatrix.m[13];
        mth_matView(&cam.view, &cam.position, &cam.target, &cam.up);

        modelViewProj = mth_matMul(&cam.view, &cam.projection);
        modelViewProj = mth_matMul(&modelMatrix, &modelViewProj);

        if(kbd_keyPressed(KEY_SPACE))
        {
            drawMode++;
            if(drawMode == MDLDRAWMODE_CNT)
                drawMode = HALF_WIREFRAME;

            if(drawMode == HALF_WIREFRAME)
                mth_matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)halfBuffer.width / halfBuffer.height, 0.1f, 500.f);
            else
                mth_matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)fullBuffer.width / fullBuffer.height, 0.1f, 500.f);

            mth_matView(&cam.view, &cam.position, &cam.target, &cam.up);
        }

        if(kbd_keyPressed(KEY_A))
        {
            if(currAnim == ALL_ANIMS)
                currAnim = -1;

            currAnim++;

            if(currAnim != ALL_ANIMS)
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
        }

        if(kbd_keyPressed(KEY_F))
            freezeFrame = !freezeFrame;

        if(kbd_keyPressed(KEY_R))
            rotating = !rotating;

        // freeze the model in the front-facing position
        if(kbd_keyDown(KEY_T))
        {
            mth_matIdentity(&modelMatrix);
            VEC4(cam.position, distFromModel, 0, 30);
            VEC4(cam.target, -1, 0, 0);
            mth_matView(&cam.view, &cam.position, &cam.target, &cam.up);
            modelViewProj = mth_matMul(&cam.view, &cam.projection);
        }
        else
        {
            VEC4(cam.position, 0, 0, 30);
        }

        if(kbd_keyDown(KEY_PLUS))
            frameTimeSkew += frameTimeSkew >= 100 ? 0 : 1;

        if(kbd_keyDown(KEY_MINUS))
            frameTimeSkew -= frameTimeSkew <= 0 ? 0 : 1;

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
                mdl_renderFrameLerp(frameNum, frameLerp * (frameTimeSkew > 0 ? 1 : 0), &mdl, &modelViewProj, &fullBuffer);
            else
                mdl_renderFrame(frameNum, &mdl, &modelViewProj, &fullBuffer);
        }

        utl_printf(&fullBuffer, 0, 1, 15, 0, "Frame : %03d/%03d @ %dx [+/-]", frameNum+1, mdl.header.num_frames, frameTimeSkew);
        utl_printf(&fullBuffer, 0, 10, 15, 0, "[A]nim: %s", animNames[currAnim]);

        gfx_updateScreen(&fullBuffer);
        gfx_vSync();

        last = now;
    } while(!kbd_keyPressed(KEY_ESC));

    mdl_free(&mdl);
    FREE_DRAWBUFFER(fullBuffer);
    FREE_DRAWBUFFER(halfBuffer);
    FREE_DRAWBUFFER(wireFrameBuffer);
}
