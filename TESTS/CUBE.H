#include "src/bitmap.h"
#include "src/camera.h"
#include "src/math.h"
#include "src/timer.h"
#include "src/triangle.h"
#include <stdio.h>

typedef struct
{
    gfx_Triangle tris[2];
} CubeTexQuad;

typedef struct
{
    CubeTexQuad walls[6];
} TexCube;

// helper functions
void setupCubeTexQuad(CubeTexQuad *q, int qx, int qy, int qw, int qh, gfx_Bitmap *texture);
void setupTexCube(TexCube *c, gfx_Bitmap *texture);
void drawCubeTexQuad(const CubeTexQuad *q, const mth_Matrix4 *mvp, gfx_drawBuffer *buffer);
void drawTexCube(const TexCube *c, const mth_Matrix4 *mvp, gfx_drawBuffer *buffer);

#define ROTATE_CUBE(delta, x, y, z) {\
            for(j = 0; j < 6; ++j) \
            { \
                for(k = 0; k < 2; ++k) \
                { \
                    for(i = 0; i < 3; i++) \
                    { \
                        mth_rotateVecAxisAngle(&cube.walls[j].tris[k].vertices[i].position, delta*dt, x, y, z); \
                    } \
                } \
            } \
        }

// Rotating cube test
void testRotatingCube()
{
    unsigned long int now, last = 0;
    const unsigned short *keysPressed;
    TexCube cube;
    int texMapFlipped = 0, cullModeFlipped = 0, depthFuncFlipped = 0;
    gfx_Camera cam;
    mth_Matrix4 modelViewProj;
    gfx_Bitmap bmp = gfx_loadBitmap("images/wood.bmp");
    gfx_drawBuffer buffer;

    ALLOC_DRAWBUFFER(buffer, SCREEN_WIDTH, SCREEN_HEIGHT, DB_COLOR | DB_DEPTH);
    buffer.drawOpts.cullMode  = FC_BACK;
    buffer.drawOpts.depthFunc = DF_LESS;

    if(!buffer.colorBuffer)
    {
        printf("Out of memory!");
        exit(1);
    }

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

    setupTexCube(&cube, &bmp);

    keysPressed = kbd_getInput();
    mth_matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)buffer.width / (float)buffer.height, 0.1f, 500.f);

    while(!keysPressed[KEY_ESC])
    {
        int i, j, k;
        float dt;
        now = tmr_getMs();
        dt = (float)(now - last);
        mth_matView(&cam.view, &cam.position, &cam.target, &cam.up);
        modelViewProj = mth_matMul(&cam.view, &cam.projection);

        if(keysPressed[KEY_RIGHT])
            ROTATE_CUBE(0.002f, 0.f, 1.f, 0.f);

        if(keysPressed[KEY_LEFT])
            ROTATE_CUBE(-0.002f, 0.f, 1.f, 0.f);

        if(keysPressed[KEY_UP])
            ROTATE_CUBE(0.002f, 1.f, 0.f, 0.f);

        if(keysPressed[KEY_DOWN])
            ROTATE_CUBE(-0.002f, 1.f, 0.f, 0.f);

        if(keysPressed[KEY_T] && !texMapFlipped)
        {
            buffer.drawOpts.texMapMode = buffer.drawOpts.texMapMode == TM_AFFINE ? TM_PERSPECTIVE : TM_AFFINE;
            texMapFlipped = 1;
            
        }
        else if(!keysPressed[KEY_T]) texMapFlipped = 0;

        if(keysPressed[KEY_C] && !cullModeFlipped)
        {
            buffer.drawOpts.cullMode = buffer.drawOpts.cullMode == FC_NONE ? FC_BACK : FC_NONE;
            cullModeFlipped = 1;
            
        }
        else if(!keysPressed[KEY_C]) cullModeFlipped = 0;

        if(keysPressed[KEY_D] && !depthFuncFlipped)
        {
            buffer.drawOpts.depthFunc = buffer.drawOpts.depthFunc == DF_ALWAYS ? DF_LESS : DF_ALWAYS;
            depthFuncFlipped = 1;
            
        }
        else if(!keysPressed[KEY_D]) depthFuncFlipped = 0;

        ROTATE_CUBE(-0.001f, 1.f, 0.f, 0.f);
        ROTATE_CUBE(-0.001f, 0.f, 1.f, 0.f);

        gfx_clrBuffer(&buffer, DB_COLOR | DB_DEPTH);
        fprintf(stdout, buffer.drawOpts.texMapMode == TM_AFFINE ? "[T]exmapping: Affine\r\n" : "[T]exmapping: Perspective\r\n");
        fprintf(stdout, buffer.drawOpts.cullMode  == FC_NONE    ? "[C]ulling BF: OFF\r\n" : "[C]ulling BF: ON\r\n");
        fprintf(stdout, buffer.drawOpts.depthFunc == DF_ALWAYS  ? "[D]epth test: OFF\r" : "[D]epth test: ON\r");
        fprintf(stdout, "%c[%d;%df", 0x1B, 0, 0);

        drawTexCube(&cube, &modelViewProj, &buffer);
        gfx_updateScreen(&buffer);

        keysPressed = kbd_getInput();
        fflush(stdout);
        last = now;
    }

    tmr_finish();
    FREE_DRAWBUFFER(buffer);
    gfx_freeBitmap(&bmp);
}


void setupCubeTexQuad(CubeTexQuad *q, int qx, int qy, int qw, int qh, gfx_Bitmap *texture)
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

#define CUBE_SIZE 15

void setupTexCube(TexCube *c, gfx_Bitmap *texture)
{
    int i, j, k;
    mth_Matrix4 wallModel;
    
    for(i = 0; i < 6; ++i)
    {
        float angle = 0.f;
        mth_Vector4 axis;
        
        axis.x = 0.f; axis.y = 0.f; axis.z = 0.f;
        setupCubeTexQuad(&c->walls[i], -CUBE_SIZE, -CUBE_SIZE, CUBE_SIZE, CUBE_SIZE, texture);
        
        mth_matIdentity(&wallModel);
        
        wallModel.m[12] = 0.f;
        wallModel.m[13] = -CUBE_SIZE;

        switch(i)
        {
            // bottom wall
            case 0: 
                wallModel.m[13] += 2*CUBE_SIZE;
                angle = -90.f;
                axis.x = 1.f;
                break;
            // top wall
            case 1: 
                angle = 90.f;
                axis.x = 1.f;
                break;
            // back wall
            case 2: 
                wallModel.m[13] += CUBE_SIZE;
                wallModel.m[14] += -CUBE_SIZE;
                angle = 180.f;
                axis.y = 1.f;
                break;
            // front wall
            case 3: 
                wallModel.m[13] += CUBE_SIZE;
                wallModel.m[14] += CUBE_SIZE;
                break;
            // right wall
            case 4: 
                wallModel.m[12] += CUBE_SIZE;
                wallModel.m[13] += CUBE_SIZE;
                angle = 90.f;
                axis.y = 1.f;
                break;
            // left wall
            case 5: 
                wallModel.m[12] += -CUBE_SIZE;
                wallModel.m[13] += CUBE_SIZE;
                angle = -90.f;
                axis.y = 1.f;
                break;
        }

        for(k = 0; k < 2; ++k)
        {
            for(j = 0; j < 3; ++j)
            {
                if(axis.x || axis.y || axis.z)
                    mth_rotateVecAxisAngle(&c->walls[i].tris[k].vertices[j].position, angle * M_PI / 180.f, axis.x, axis.y, axis.z);

                c->walls[i].tris[k].vertices[j].position = mth_matMulVec(&wallModel, &c->walls[i].tris[k].vertices[j].position);
            }
        } 
    }
}

void drawCubeTexQuad(const CubeTexQuad *q, const mth_Matrix4 *mvp, gfx_drawBuffer *buffer)
{
    gfx_drawTriangle(&q->tris[0], mvp, buffer);
    gfx_drawTriangle(&q->tris[1], mvp, buffer);
}

void drawTexCube(const TexCube *c, const mth_Matrix4 *mvp, gfx_drawBuffer *buffer)
{
    int i;

    for(i = 0; i < 6; ++i)
        drawCubeTexQuad(&c->walls[i], mvp, buffer);
}
