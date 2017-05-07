#include "src/camera.h"
#include "src/triangle.h"
#include <dos.h>
#include <stdlib.h>

typedef struct
{
    int color;
    gfx_Triangle tris[2];
} Quad;

void setupTestQuad(Quad *q, int qx, int qy, int qw, int qh);

// Quad rendering test
void testQuads()
{
    const unsigned short *keysPressed;
    gfx_Camera cam;
    int i,k , paused = 0, numQuads = 0;
    Quad quad, screenQuad;
    mth_Matrix4 model;
    mth_Matrix4 modelViewProj;
    unsigned char *buffer = (unsigned char *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT);

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
    cam.position.z = 150.f;
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

    // translate view to screen center
    model.m[12] = -160.f;
    model.m[13] = -100.f;

    keysPressed = kbd_getInput();
    gfx_clrBufferColor(buffer, 107);

    while(!keysPressed[KEY_ESC])
    {
        int qx = rand() % 320;
        int qy = rand() % 200;
        int qw = (qx + rand()) % 320;
        int qh = (qy + rand()) % 200;

        quad.color = rand() % 103 + 1;
        setupTestQuad(&quad, qx, qy, qw, qh );

        screenQuad.color = quad.color;
        screenQuad.tris[0] = quad.tris[0];
        screenQuad.tris[1] = quad.tris[1];
        
        mth_matView(&cam.view, &cam.position, &cam.target, &cam.up);
        mth_matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 5.f);
        modelViewProj = mth_matMul(&cam.view, &cam.projection);
        modelViewProj = mth_matMul(&model, &modelViewProj);

        for(k = 0; k < 2; k++)
        {
            for(i = 0; i < 3; i++)
            {
                screenQuad.tris[k].vertices[i].position = mth_matMulVec(&modelViewProj, &quad.tris[k].vertices[i].position);
                screenQuad.tris[k].vertices[i].position.x = (screenQuad.tris[k].vertices[i].position.x * (float)SCREEN_WIDTH)  / (2.0f * screenQuad.tris[k].vertices[i].position.w) + (SCREEN_WIDTH >> 1);
                screenQuad.tris[k].vertices[i].position.y = (screenQuad.tris[k].vertices[i].position.y * (float)SCREEN_HEIGHT) / (2.0f * screenQuad.tris[k].vertices[i].position.w) + (SCREEN_HEIGHT >> 1);
            }
        }

        if(paused == 0)
        {
            gfx_drawTriangle(&screenQuad.tris[0], buffer);
            gfx_drawTriangle(&screenQuad.tris[1], buffer);
            gfx_updateScreen(buffer);
            fprintf(stdout, "Quads rendered: %d\r", ++numQuads);
            fflush(stdout);
            delay(50);
        }

        keysPressed = kbd_getInput();

        // pause rendering when holding down space bar
        paused = keysPressed[KEY_SPACE];
    }

    free(buffer);
}

void setupTestQuad(Quad *q, int qx, int qy, int qw, int qh)
{
    q->tris[0].color = q->color;
    q->tris[0].texture = NULL;
    q->tris[0].vertices[0].position.x = qx;
    q->tris[0].vertices[0].position.y = qh;
    q->tris[0].vertices[0].position.z = 0;
    q->tris[0].vertices[0].position.w = 1.f;
    q->tris[0].vertices[1].position.x = qw;
    q->tris[0].vertices[1].position.y = qy;
    q->tris[0].vertices[1].position.z = 0;
    q->tris[0].vertices[1].position.w = 1.f;
    q->tris[0].vertices[2].position.x = qx;
    q->tris[0].vertices[2].position.y = qy;
    q->tris[0].vertices[2].position.z = 0;
    q->tris[0].vertices[2].position.w = 1.f;

    q->tris[1].color = q->color;
    q->tris[1].texture = NULL;
    q->tris[1].vertices[0].position.x = qx;
    q->tris[1].vertices[0].position.y = qh;
    q->tris[1].vertices[0].position.z = 0;
    q->tris[1].vertices[0].position.w = 1.f;
    q->tris[1].vertices[1].position.x = qw;
    q->tris[1].vertices[1].position.y = qh;
    q->tris[1].vertices[1].position.z = 0;
    q->tris[1].vertices[1].position.w = 1.f;
    q->tris[1].vertices[2].position.x = qw;
    q->tris[1].vertices[2].position.y = qy;
    q->tris[1].vertices[2].position.z = 0;
    q->tris[1].vertices[2].position.w = 1.f;
}
