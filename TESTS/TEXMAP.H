#include "src/bitmap.h"
#include "src/camera.h"
#include "src/triangle.h"
#include "src/math.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    gfx_Triangle tris[2];
} TexQuad;

// helper functions
void setupTexQuad(TexQuad *q, int qx, int qy, int qw, int qh, gfx_Bitmap *texture);
void drawTexQuad(const TexQuad *q, const mth_Matrix4 *mvp, const gfx_drawOptions *drawOpts, unsigned char *buffer);

// Texture mapping test
void testTextureMapping()
{
    const unsigned short *keysPressed;
    TexQuad quad;
    int texMapFlipped = 0;
    gfx_Camera cam;
    gfx_drawOptions drawOpts;
    mth_Matrix4 model;
    mth_Matrix4 modelViewProj;
    gfx_Bitmap bmp = gfx_loadBitmap("images/wood.bmp");
    unsigned char *buffer = (unsigned char *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT);

    DRAWOPTS_DEFAULT(drawOpts);

    if(!buffer)
    {
        printf("Out of memory!");
        exit(1);
    }

    mth_matIdentity(&model);
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

    // additional camera translation
    //model.m[12] = -20.f;
    //model.m[13] = -20.f;

    setupTexQuad(&quad, -20, -20, 20, 20, &bmp);

    keysPressed = kbd_getInput();

    while(!keysPressed[KEY_ESC])
    {
        int i, k;
        mth_matView(&cam.view, &cam.position, &cam.target, &cam.up);
        mth_matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 500.f);
        modelViewProj = mth_matMul(&cam.view, &cam.projection);
        modelViewProj = mth_matMul(&model, &modelViewProj);

        if(keysPressed[KEY_A])
        {
            for(k = 0; k < 2; ++k)
            {
                for(i = 0; i < 3; i++)
                {
                    mth_rotateVecAxisAngle(&quad.tris[k].vertices[i].position, 0.01f, 0.f, 1.f, 0.f);
                }
            }
        }

        if(keysPressed[KEY_D])
        {
            for(k = 0; k < 2; ++k)
            {
                for(i = 0; i < 3; i++)
                {
                    mth_rotateVecAxisAngle(&quad.tris[k].vertices[i].position, -0.01f, 0.f, 1.f, 0.f);
                }
            }
        }

        if(keysPressed[KEY_W])
        {
            for(k = 0; k < 2; ++k)
            {
                for(i = 0; i < 3; i++)
                {
                    mth_rotateVecAxisAngle(&quad.tris[k].vertices[i].position, 0.01f, 1.f, 0.f, 0.f);
                }
            }
        }

        if(keysPressed[KEY_S])
        {
            for(k = 0; k < 2; ++k)
            {
                for(i = 0; i < 3; i++)
                {
                    mth_rotateVecAxisAngle(&quad.tris[k].vertices[i].position, -0.01f, 1.f, 0.f, 0.f);
                }
            }
        }

        if(keysPressed[KEY_G] && !texMapFlipped)
        {
            drawOpts.texMapMode = drawOpts.texMapMode == AFFINE ? PERSPECTIVE : AFFINE;
            texMapFlipped = 1;
            
        }
        else if(!keysPressed[KEY_G]) texMapFlipped = 0;

        gfx_clrBuffer(buffer);
        fprintf(stdout, drawOpts.texMapMode == AFFINE ? "Affine\r" : "Perspective\r");

        drawTexQuad(&quad, &modelViewProj, &drawOpts, buffer);
        gfx_updateScreen(buffer);

        keysPressed = kbd_getInput();
        fflush(stdout);
    }

    free(buffer);
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

void drawTexQuad(const TexQuad *q, const mth_Matrix4 *mvp, const gfx_drawOptions *drawOpts, unsigned char *buffer)
{
    gfx_drawTriangleOpts(&q->tris[0], mvp, drawOpts, buffer);
    gfx_drawTriangleOpts(&q->tris[1], mvp, drawOpts, buffer);
}
