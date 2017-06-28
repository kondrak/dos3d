#include "src/bitmap.h"
#include "src/camera.h"
#include "src/math.h"
#include "src/timer.h"
#include "src/triangle.h"
#include <stdio.h>

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
    unsigned long int now, last = 0;
    const unsigned short *keysPressed;
    TexQuad quad;
    int texMapFlipped = 0, depthFuncFlipped = 0;
    gfx_Camera cam;
    mth_Matrix4 modelViewProj;
    gfx_Bitmap bmp = gfx_loadBitmap("images/quake.bmp");
    gfx_drawBuffer buffer;

    ALLOC_DRAWBUFFER(buffer, SCREEN_WIDTH, SCREEN_HEIGHT, DB_COLOR | DB_DEPTH);
    buffer.drawOpts.depthFunc = DF_ALWAYS;

    if(!DRAWBUFFER_VALID(buffer, DB_COLOR | DB_DEPTH))
    {
        printf("Out of memory!");
        exit(1);
    }

    gfx_setPalette(bmp.palette);

    tmr_start();

    mth_matIdentity(&modelViewProj);

    // setup camera
    VEC4(cam.position, 0, 0, 60);
    VEC4(cam.up, 0, 1, 0);
    VEC4(cam.right, 1, 0, 0);
    VEC4(cam.target, 0, 0, -1);

    // setup textured quad
    setupTexQuad(&quad, -25, -25, 25, 25, &bmp);

    mth_matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)buffer.width / (float)buffer.height, 0.1f, 500.f);
    mth_matView(&cam.view, &cam.position, &cam.target, &cam.up);
    modelViewProj = mth_matMul(&cam.view, &cam.projection);

    do
    {
        int i, k;
        float dt;
        now = tmr_getMs();
        dt = (float)(now - last);
        keysPressed = kbd_getInput();

        if(keysPressed[KEY_RIGHT])
            ROTATE_QUAD(0.002f, 0.f, 1.f, 0.f);

        if(keysPressed[KEY_LEFT])
            ROTATE_QUAD(-0.002f, 0.f, 1.f, 0.f);

        if(keysPressed[KEY_UP])
            ROTATE_QUAD(0.002f, 1.f, 0.f, 0.f);

        if(keysPressed[KEY_DOWN])
            ROTATE_QUAD(-0.002f, 1.f, 0.f, 0.f);

        if(keysPressed[KEY_T] && !texMapFlipped)
        {
            buffer.drawOpts.drawMode = buffer.drawOpts.drawMode == DM_AFFINE ? DM_PERSPECTIVE : DM_AFFINE;
            texMapFlipped = 1;
            
        }
        else if(!keysPressed[KEY_T]) texMapFlipped = 0;

        if(keysPressed[KEY_D] && !depthFuncFlipped)
        {
            buffer.drawOpts.depthFunc = buffer.drawOpts.depthFunc == DF_ALWAYS ? DF_LESS : DF_ALWAYS;
            depthFuncFlipped = 1;
            
        }
        else if(!keysPressed[KEY_D]) depthFuncFlipped = 0;

        // clear buffers and draw the quad!
        gfx_clrBuffer(&buffer, DB_COLOR | DB_DEPTH);
        drawTexQuad(&quad, &modelViewProj, &buffer);
        gfx_updateScreen(&buffer);

        fprintf(stdout, buffer.drawOpts.drawMode == DM_AFFINE ? "[T]exmapping: Affine\r\n" : "[T]exmapping: Perspective\r\n");
        fprintf(stdout, buffer.drawOpts.depthFunc == DF_ALWAYS  ? "[D]epth test: OFF\r" : "[D]epth test: ON\r");
        // reset carriage to top left corner of the screen
        fprintf(stdout, "%c[%d;%df", 0x1B, 0, 0);        
        gfx_vSync();

        keysPressed = kbd_getInput();
        fflush(stdout);
        last = now;
    } while(!keysPressed[KEY_ESC]);

    tmr_finish();
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
