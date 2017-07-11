#include "src/bitmap.h"
#include "src/camera.h"
#include "src/math.h"
#include "src/timer.h"
#include "src/triangle.h"
#include "src/utils.h"

typedef struct
{
    gfx_Triangle tris[2];
} CubeTexQuad;

typedef struct
{
    CubeTexQuad walls[6];
} TexCube;

enum DrawModeType
{
    TEX_CUBE,     // draw textured cube
    DEPTH_VALUES, // draw depth buffer values
    FLAT_COLOR,   // draw flat shaded cube
    DRAWMODE_CNT
};

// helper functions
void setupCubeTexQuad(CubeTexQuad *q, int qx, int qy, int qw, int qh, uint8_t color, gfx_Bitmap *texture);
void setupTexCube(TexCube *c, gfx_Bitmap *texture);
void drawCubeTexQuad(const CubeTexQuad *q, const mth_Matrix4 *mvp, gfx_drawBuffer *buffer);
void drawTexCube(const TexCube *c, const mth_Matrix4 *mvp, gfx_drawBuffer *buffer);

#define ROTATE_CUBE(delta, x, y, z) {\
            for(j = 0; j < 6; ++j) \
            { \
                for(k = 0; k < 2; ++k) \
                { \
                    for(i = 0; i < 3; ++i) \
                    { \
                        mth_rotateVecAxisAngle(&cube.walls[j].tris[k].vertices[i].position, delta*dt, x, y, z); \
                    } \
                } \
            } \
        }

// Rotating cube test
void testRotatingCube()
{
    uint32_t dt, now, last = tmr_getMs();
    uint8_t defaultPalette[256*3], grayScalePalette[256*3];
    TexCube cube;
    int i, rotating = 1, drawMode = TEX_CUBE;
    gfx_Camera cam;
    mth_Matrix4 modelViewProj;
    gfx_Bitmap bmp = gfx_loadBitmap("images/wood.bmp");
    gfx_drawBuffer buffer, depthDebug;

    // main draw buffer where the cube is rendered
    ALLOC_DRAWBUFFER(buffer, SCREEN_WIDTH, SCREEN_HEIGHT, DB_COLOR | DB_DEPTH);
    ASSERT(DRAWBUFFER_VALID(buffer, DB_COLOR | DB_DEPTH), "Out of memory!\n");
    buffer.drawOpts.cullMode  = FC_BACK;
    buffer.drawOpts.depthFunc = DF_LESS;

    // secondary buffer to render depth buffer values
    ALLOC_DRAWBUFFER(depthDebug, SCREEN_WIDTH, SCREEN_HEIGHT, DB_COLOR);
    ASSERT(DRAWBUFFER_VALID(depthDebug, DB_COLOR), "Out of memory!\n");

    // setup camera
    VEC4(cam.position, 0, 0, 60);
    VEC4(cam.up, 0, 1, 0);
    VEC4(cam.right, 1, 0, 0);
    VEC4(cam.target, 0, 0, -1);

    // setup the cube
    setupTexCube(&cube, &bmp);

    // save default palette and generate pseudo-grayscale color set for depth buffer rendering
    gfx_getPalette(defaultPalette);
    for(i = 0; i < 768; i += 3)
    {
        grayScalePalette[i] = i/3;
        grayScalePalette[i+1] = i/3;
        grayScalePalette[i+2] = i/3;
    }
    gfx_setPalette(bmp.palette);

    mth_matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)buffer.width / buffer.height, 0.1f, 500.f);
    mth_matView(&cam.view, &cam.position, &cam.target, &cam.up);
    modelViewProj = mth_matMul(&cam.view, &cam.projection);

    do
    {
        int j, k;
        now = tmr_getMs();
        dt  = now - last;

        // auto-rotation of the cube
        if(rotating)
        {
            ROTATE_CUBE(-0.001f, 1.f, 0.f, 0.f);
            ROTATE_CUBE(-0.001f, 0.f, 1.f, 0.f);
        }

        if(kbd_keyDown(KEY_RIGHT))
            ROTATE_CUBE(0.002f, 0.f, 1.f, 0.f);

        if(kbd_keyDown(KEY_LEFT))
            ROTATE_CUBE(-0.002f, 0.f, 1.f, 0.f);

        if(kbd_keyDown(KEY_UP))
            ROTATE_CUBE(0.002f, 1.f, 0.f, 0.f);

        if(kbd_keyDown(KEY_DOWN))
            ROTATE_CUBE(-0.002f, 1.f, 0.f, 0.f);

        if(kbd_keyPressed(KEY_T))
            buffer.drawOpts.drawMode ^= (DM_PERSPECTIVE | DM_AFFINE);

        if(kbd_keyPressed(KEY_W))
            buffer.drawOpts.drawMode ^= DM_WIREFRAME;

        if(kbd_keyPressed(KEY_C))
            buffer.drawOpts.cullMode = buffer.drawOpts.cullMode == FC_NONE ? FC_BACK : FC_NONE;

        if(kbd_keyPressed(KEY_D) && drawMode != DEPTH_VALUES)
            buffer.drawOpts.depthFunc = buffer.drawOpts.depthFunc == DF_ALWAYS ? DF_LESS : DF_ALWAYS;

        if(kbd_keyPressed(KEY_R))
            rotating = !rotating;

        if(kbd_keyPressed(KEY_SPACE))
        {
            drawMode++;
            if(drawMode == DRAWMODE_CNT)
                drawMode = TEX_CUBE;

            // use image palette in textured cube
            if(drawMode == TEX_CUBE)
            {
                gfx_setPalette(bmp.palette);
                buffer.drawOpts.drawMode &= ~DM_FLAT;
            }

            // viewing depth values requires depth testing to be enabled
            if(drawMode == DEPTH_VALUES)
                buffer.drawOpts.depthFunc = DF_LESS;

            // for flat color reset to default palette
            if(drawMode == FLAT_COLOR)
            {
                gfx_setPalette(defaultPalette);
                buffer.drawOpts.drawMode |= DM_FLAT;
            }
        }

        // clear screen and render the cube!
        gfx_clrBuffer(&buffer, DB_COLOR | DB_DEPTH);
        drawTexCube(&cube, &modelViewProj, &buffer);

        if(drawMode == DEPTH_VALUES)
        {
            gfx_setPalette(grayScalePalette);

            // fill color buffer with depth buffer values and render it in grayscale
            gfx_clrBufferColor(&depthDebug, 0x80);
            for(i = 0; i < buffer.width * buffer.height; ++i)
            {
                if(buffer.depthBuffer[i])
                {
                    uint8_t c = 2.f / buffer.depthBuffer[i];
                    depthDebug.colorBuffer[i] = 0x80 - (c > 0x7B ? 0x7B : c);
                }
            }

            utl_printf(&depthDebug, 0, 1, 48, 0, "1/Z depth values");
            gfx_updateScreen(&depthDebug);
        }
        else
        {
            utl_printf(&buffer, 0,  1, 15, 0, "[T]exmapping: %s", buffer.drawOpts.drawMode & DM_AFFINE ? "Affine" : "Perspective");
            utl_printf(&buffer, 0, 10, 15, 0, "[C]ulling BF: %s", buffer.drawOpts.cullMode  == FC_NONE ? "OFF" : "ON");
            utl_printf(&buffer, 0, 19, 15, 0, "[D]epth test: %s", buffer.drawOpts.depthFunc == DF_ALWAYS ? "OFF" : "ON");
            utl_printf(&buffer, 0, 28, 15, 0, "[W]ireframe : %s", buffer.drawOpts.drawMode & DM_WIREFRAME ? "ON"  : "OFF");
            gfx_updateScreen(&buffer);
        }

        gfx_vSync();

        last = now;
    } while(!kbd_keyPressed(KEY_ESC));

    FREE_DRAWBUFFER(buffer);
    FREE_DRAWBUFFER(depthDebug);
    gfx_freeBitmap(&bmp);
}

/* ***** */
void setupCubeTexQuad(CubeTexQuad *q, int qx, int qy, int qw, int qh, uint8_t color, gfx_Bitmap *texture)
{
    q->tris[0].color = color;
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

    q->tris[1].color = color;
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

#define CUBE_SIZE 15
/* ***** */
void setupTexCube(TexCube *c, gfx_Bitmap *texture)
{
    int i, j, k;
    mth_Matrix4 wallModel;
    
    for(i = 0; i < 6; ++i)
    {
        float angle = 0.f;
        mth_Vector4 axis;
        
        axis.x = 0.f; axis.y = 0.f; axis.z = 0.f;
        setupCubeTexQuad(&c->walls[i], -CUBE_SIZE, -CUBE_SIZE, CUBE_SIZE, CUBE_SIZE, i+1, texture);
        
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

/* ***** */
void drawCubeTexQuad(const CubeTexQuad *q, const mth_Matrix4 *mvp, gfx_drawBuffer *buffer)
{
    gfx_drawTriangle(&q->tris[0], mvp, buffer);
    gfx_drawTriangle(&q->tris[1], mvp, buffer);
}

/* ***** */
void drawTexCube(const TexCube *c, const mth_Matrix4 *mvp, gfx_drawBuffer *buffer)
{
    int i;
    for(i = 0; i < 6; ++i)
        drawCubeTexQuad(&c->walls[i], mvp, buffer);
}
