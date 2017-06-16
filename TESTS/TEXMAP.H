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
                for(i = 0; i < 3; i++) \
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
    gfx_Bitmap bmp = gfx_loadBitmap("images/wood.bmp");
    gfx_drawBuffer buffer;

    ALLOC_DRAWBUFFER(buffer, SCREEN_WIDTH, SCREEN_HEIGHT, DB_COLOR | DB_DEPTH);
    buffer.drawOpts.depthFunc = DF_LESS;

    if(!buffer.colorBuffer)
    {
        printf("Out of memory!");
        exit(1);
    }

    gfx_setPalette(bmp.palette);

    tmr_start();

    mth_matIdentity(&modelViewProj);

    // setup camera
    cam.position.x = 0;
    cam.position.y = 0;
    cam.position.z = 60.f;
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

    setupTexQuad(&quad, -20, -20, 20, 20, &bmp);

    keysPressed = kbd_getInput();
    mth_matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)buffer.width / (float)buffer.height, 0.1f, 500.f);

    while(!keysPressed[KEY_ESC])
    {
        int i, k;
        float dt;
        now = tmr_getMs();
        dt = (float)(now - last);
        mth_matView(&cam.view, &cam.position, &cam.target, &cam.up);
        modelViewProj = mth_matMul(&cam.view, &cam.projection);

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
            buffer.drawOpts.texMapMode = buffer.drawOpts.texMapMode == TM_AFFINE ? TM_PERSPECTIVE : TM_AFFINE;
            texMapFlipped = 1;
            
        }
        else if(!keysPressed[KEY_T]) texMapFlipped = 0;

        if(keysPressed[KEY_D] && !depthFuncFlipped)
        {
            buffer.drawOpts.depthFunc = buffer.drawOpts.depthFunc == DF_ALWAYS ? DF_LESS : DF_ALWAYS;
            depthFuncFlipped = 1;
            
        }
        else if(!keysPressed[KEY_D]) depthFuncFlipped = 0;

        gfx_clrBuffer(&buffer, DB_COLOR | DB_DEPTH);
        fprintf(stdout, buffer.drawOpts.texMapMode == TM_AFFINE ? "[T]exmapping: Affine\r\n" : "[T]exmapping: Perspective\r\n");
        fprintf(stdout, buffer.drawOpts.depthFunc == DF_ALWAYS  ? "[D]epth test: OFF\r" : "[D]epth test: ON\r");
        fprintf(stdout, "%c[%d;%df", 0x1B, 0, 0);

        drawTexQuad(&quad, &modelViewProj, &buffer);
        gfx_updateScreen(&buffer);
        gfx_vSync();

        keysPressed = kbd_getInput();
        fflush(stdout);
        last = now;
    }

    tmr_finish();
    FREE_DRAWBUFFER(buffer);
    gfx_freeBitmap(&bmp);
}


void setupTexQuad(TexQuad *q, int qx, int qy, int qw, int qh, gfx_Bitmap *texture)
{
    q->tris[0].color = 1;
    q->tris[0].texture = texture;
    q->tris[0].vertices[0].position.x = qx;
    q->tris[0].vertices[0].position.y = qh;
    q->tris[0].vertices[0].position.z = 0;
    q->tris[0].vertices[0].position.w = 1.f;
    q->tris[0].vertices[0].uv.u = 0;
    q->tris[0].vertices[0].uv.v = 1;
    q->tris[0].vertices[1].position.x = qw;
    q->tris[0].vertices[1].position.y = qy;
    q->tris[0].vertices[1].position.z = 0;
    q->tris[0].vertices[1].position.w = 1.f;
    q->tris[0].vertices[1].uv.u = 1;
    q->tris[0].vertices[1].uv.v = 0;
    q->tris[0].vertices[2].position.x = qx;
    q->tris[0].vertices[2].position.y = qy;
    q->tris[0].vertices[2].position.z = 0;
    q->tris[0].vertices[2].position.w = 1.f;
    q->tris[0].vertices[2].uv.u = 0;
    q->tris[0].vertices[2].uv.v = 0;

    q->tris[1].color = 1;
    q->tris[1].texture = texture;
    q->tris[1].vertices[0].position.x = qx;
    q->tris[1].vertices[0].position.y = qh;
    q->tris[1].vertices[0].position.z = 0;
    q->tris[1].vertices[0].position.w = 1.f;
    q->tris[1].vertices[0].uv.u = 0;
    q->tris[1].vertices[0].uv.v = 1;
    q->tris[1].vertices[1].position.x = qw;
    q->tris[1].vertices[1].position.y = qh;
    q->tris[1].vertices[1].position.z = 0;
    q->tris[1].vertices[1].position.w = 1.f;
    q->tris[1].vertices[1].uv.u = 1;
    q->tris[1].vertices[1].uv.v = 1;
    q->tris[1].vertices[2].position.x = qw;
    q->tris[1].vertices[2].position.y = qy;
    q->tris[1].vertices[2].position.z = 0;
    q->tris[1].vertices[2].position.w = 1.f;
    q->tris[1].vertices[2].uv.u = 1;
    q->tris[1].vertices[2].uv.v = 0;
}

void drawTexQuad(const TexQuad *q, const mth_Matrix4 *mvp, gfx_drawBuffer *buffer)
{
    gfx_drawTriangle(&q->tris[0], mvp, buffer);
    gfx_drawTriangle(&q->tris[1], mvp, buffer);
}
