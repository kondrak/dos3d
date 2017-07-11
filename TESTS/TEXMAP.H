#include "src/bitmap.h"
#include "src/camera.h"
#include "src/math.h"
#include "src/timer.h"
#include "src/triangle.h"
#include "src/utils.h"

typedef struct
{
    gfx_Triangle tris[2];
} TexQuad;

// helper functions
void setupTexQuad(TexQuad *q, int qx, int qy, int qw, int qh, gfx_Bitmap *texture);
void drawTexQuad(const TexQuad *q, const mth_Matrix4 *mvp, gfx_drawBuffer *buffer);

#define ROTATE_QUAD(delta, x, y, z) {\
            for(k = 0; k < 2; ++k) \
            { \
                for(i = 0; i < 3; ++i) \
                { \
                    mth_rotateVecAxisAngle(&quad.tris[k].vertices[i].position, delta*dt, x, y, z); \
                } \
            } \
        }

// Texture mapping test
void testTextureMapping()
{
    const int quadX = -25;
    const int quadY = -25;
    const int quadW =  25;
    const int quadH =  25;
    uint32_t dt, now, last = tmr_getMs();
    float t = 0.f;
    TexQuad quad;
    int manualMode = 0;
    gfx_Camera cam;
    mth_Matrix4 modelViewProj;
    gfx_Bitmap bmp = gfx_loadBitmap("images/quake.bmp");
    gfx_drawBuffer buffer;

    ALLOC_DRAWBUFFER(buffer, SCREEN_WIDTH, SCREEN_HEIGHT, DB_COLOR);
    ASSERT(DRAWBUFFER_VALID(buffer, DB_COLOR), "Out of memory!\n");
    // use proper palette
    gfx_setPalette(bmp.palette);

    // setup camera
    VEC4(cam.position, 0, 0, 60);
    VEC4(cam.up, 0, 1, 0);
    VEC4(cam.right, 1, 0, 0);
    VEC4(cam.target, 0, 0, -1);

    // setup textured quad
    setupTexQuad(&quad, quadX, quadY, quadW, quadH, &bmp);

    mth_matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)buffer.width / buffer.height, 0.1f, 500.f);
    mth_matView(&cam.view, &cam.position, &cam.target, &cam.up);
    modelViewProj = mth_matMul(&cam.view, &cam.projection);

    do
    {
        int i, k;
        now = tmr_getMs();
        dt  = now - last;
        t  += 0.003f * dt;

        if(manualMode)
        {
            if(kbd_keyDown(KEY_RIGHT))
                ROTATE_QUAD(0.002f, 0.f, 1.f, 0.f);

            if(kbd_keyDown(KEY_LEFT))
                ROTATE_QUAD(-0.002f, 0.f, 1.f, 0.f);

            if(kbd_keyDown(KEY_UP))
                ROTATE_QUAD(0.002f, 1.f, 0.f, 0.f);

            if(kbd_keyDown(KEY_DOWN))
                ROTATE_QUAD(-0.002f, 1.f, 0.f, 0.f);
        }
        else
            ROTATE_QUAD(0.002f * cos(t), 0.f, 1.f, 0.f);

        if(kbd_keyPressed(KEY_T))
            buffer.drawOpts.drawMode = buffer.drawOpts.drawMode == DM_AFFINE ? DM_PERSPECTIVE : DM_AFFINE;

        if(kbd_keyPressed(KEY_SPACE))
        {
            setupTexQuad(&quad, quadX, quadY, quadW, quadH, &bmp);
            manualMode = !manualMode;
            t = 0.f;
        }

        // clear buffers and draw the quad!
        gfx_clrBuffer(&buffer, DB_COLOR);
        drawTexQuad(&quad, &modelViewProj, &buffer);

        utl_printf(&buffer, 0,  1, 253, 0, "[T]exmapping: %s", buffer.drawOpts.drawMode  == DM_AFFINE ? "Affine" : "Perspective");
        gfx_updateScreen(&buffer);
        gfx_vSync();

        last = now;
    } while(!kbd_keyPressed(KEY_ESC));

    FREE_DRAWBUFFER(buffer);
    gfx_freeBitmap(&bmp);
}

/* ***** */
void setupTexQuad(TexQuad *q, int qx, int qy, int qw, int qh, gfx_Bitmap *texture)
{
    q->tris[0].color = 1;
    q->tris[0].texture = texture;
    VEC4(q->tris[0].vertices[0].position, qx, qh, 0);
    q->tris[0].vertices[0].uv.u = 0;
    q->tris[0].vertices[0].uv.v = 1;
    VEC4(q->tris[0].vertices[1].position, qw, qy, 0);
    q->tris[0].vertices[1].uv.u = 1;
    q->tris[0].vertices[1].uv.v = 0;
    VEC4(q->tris[0].vertices[2].position, qx, qy, 0);
    q->tris[0].vertices[2].uv.u = 0;
    q->tris[0].vertices[2].uv.v = 0;

    q->tris[1].color = 1;
    q->tris[1].texture = texture;
    VEC4(q->tris[1].vertices[0].position, qx, qh, 0);
    q->tris[1].vertices[0].uv.u = 0;
    q->tris[1].vertices[0].uv.v = 1;
    VEC4(q->tris[1].vertices[1].position, qw, qh, 0);
    q->tris[1].vertices[1].uv.u = 1;
    q->tris[1].vertices[1].uv.v = 1;
    VEC4(q->tris[1].vertices[2].position, qw, qy, 0);
    q->tris[1].vertices[2].uv.u = 1;
    q->tris[1].vertices[2].uv.v = 0;
}

/* ***** */
void drawTexQuad(const TexQuad *q, const mth_Matrix4 *mvp, gfx_drawBuffer *buffer)
{
    gfx_drawTriangle(&q->tris[0], mvp, buffer);
    gfx_drawTriangle(&q->tris[1], mvp, buffer);
}
