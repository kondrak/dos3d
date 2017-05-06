#include "src/bitmap.h"
#include "src/camera.h"
#include "src/triangle.h"
#include "src/math.h"
#include <stdlib.h>

typedef struct
{
    Bitmap *texture;
    Triangle tris[2];
} TexQuad;

// helper functions
void setupTexQuad(TexQuad *q, int qx, int qy, int qw, int qh);
void drawTexQuad(TexQuad *q, unsigned char *buffer);

// Texture mapping test
void testTextureMapping()
{
    unsigned short *keysPressed;
    TexQuad quad, screenQuad;
    Camera cam;
    Matrix4f model;
    Matrix4f modelViewProj;
    Bitmap bmp = loadBitmap("images/wood.bmp");
    unsigned char *buffer = (unsigned char *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT);

    if(!buffer)
    {
        printf("Out of memory!");
        return;
    }

    matIdentity(&model);
    matIdentity(&modelViewProj);

    // setup camera
    cam.position.x = 0;
    cam.position.y = 0;
    cam.position.z = 100.f;
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

    quad.texture = &bmp;
    setupTexQuad(&quad, -20, -20, 20, 20);

    screenQuad.texture = quad.texture;
    screenQuad.tris[0] = quad.tris[0];
    screenQuad.tris[1] = quad.tris[1];

    keysPressed = translateInput();

    while(!keysPressed[KEY_ESC])
    {
        int i, k;
        matView(&cam.view, &cam.position, &cam.target, &cam.up);
        matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 5.f);
        modelViewProj = matMul(&cam.view, &cam.projection);
        modelViewProj = matMul(&model, &modelViewProj);

        if(keysPressed[KEY_A])
        {
            for(k = 0; k < 2; ++k)
            {
                for(i = 0; i < 3; i++)
                {
                    rotateVecAxisAngle(&quad.tris[k].vertices[i].position, 0.01f, 0.f, 1.f, 0.f);
                }
            }
        }

        if(keysPressed[KEY_D])
        {
            for(k = 0; k < 2; ++k)
            {
                for(i = 0; i < 3; i++)
                {
                    rotateVecAxisAngle(&quad.tris[k].vertices[i].position, -0.01f, 0.f, 1.f, 0.f);
                }
            }
        }

        if(keysPressed[KEY_W])
        {
            for(k = 0; k < 2; ++k)
            {
                for(i = 0; i < 3; i++)
                {
                    rotateVecAxisAngle(&quad.tris[k].vertices[i].position, 0.01f, 1.f, 0.f, 0.f);
                }
            }
        }

        if(keysPressed[KEY_S])
        {
            for(k = 0; k < 2; ++k)
            {
                for(i = 0; i < 3; i++)
                {
                    rotateVecAxisAngle(&quad.tris[k].vertices[i].position, -0.01f, 1.f, 0.f, 0.f);
                }
            }
        }

        for(k = 0; k < 2; k++)
        {
            for(i = 0; i < 3; i++)
            {
                screenQuad.tris[k].vertices[i].position = matMulVec(&modelViewProj, &quad.tris[k].vertices[i].position);
                screenQuad.tris[k].vertices[i].position.x = (screenQuad.tris[k].vertices[i].position.x * (float)SCREEN_WIDTH)  / (2.0f * screenQuad.tris[k].vertices[i].position.w) + (SCREEN_WIDTH >> 1);
                screenQuad.tris[k].vertices[i].position.y = (screenQuad.tris[k].vertices[i].position.y * (float)SCREEN_HEIGHT) / (2.0f * screenQuad.tris[k].vertices[i].position.w) + (SCREEN_HEIGHT >> 1);
            }
        }

        clrScrBuffer(buffer);
        drawTexQuad(&screenQuad, buffer);
        updateScreen(buffer);

        keysPressed = translateInput();
    }

    free(buffer);
    freeBitmap(&bmp);

    do {
        keysPressed = translateInput();
    } while(keysPressed[KEY_ESC]);
}


void setupTexQuad(TexQuad *q, int qx, int qy, int qw, int qh)
{
    q->tris[0].color = 1;
    q->tris[0].texture = q->texture;
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
    q->tris[1].texture = q->texture;
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

void drawTexQuad(TexQuad *q, unsigned char *buffer)
{
    drawTriangle(&q->tris[0], buffer);
    drawTriangle(&q->tris[1], buffer);
}
