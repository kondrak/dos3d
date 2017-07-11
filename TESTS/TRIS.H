#include "src/camera.h"
#include "src/triangle.h"
#include "src/utils.h"
#include <dos.h>

// Triangle rendering test
void testTriangles()
{
    gfx_Camera cam;
    int numTris = 0;
    gfx_Triangle triangle;
    mth_Matrix4 model;
    mth_Matrix4 modelViewProj;
    gfx_drawBuffer buffer;

    ALLOC_DRAWBUFFER(buffer, SCREEN_WIDTH, SCREEN_HEIGHT, DB_COLOR);
    ASSERT(DRAWBUFFER_VALID(buffer, DB_COLOR), "Out of memory!\n");

    // setup camera
    VEC4(cam.position, 0, 0, 150);
    VEC4(cam.up, 0, 1, 0);
    VEC4(cam.right, 1, 0, 0);
    VEC4(cam.target, 0, 0, -1);

    // translate view to screen center
    mth_matIdentity(&model);
    model.m[12] = -160.f;
    model.m[13] = -100.f;

    gfx_clrBufferColor(&buffer, 105);

    mth_matView(&cam.view, &cam.position, &cam.target, &cam.up);
    mth_matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)buffer.width / buffer.height, 0.1f, 500.f);
    modelViewProj = mth_matMul(&cam.view, &cam.projection);
    modelViewProj = mth_matMul(&model, &modelViewProj);
    triangle.texture = NULL;

    do
    {
        // render randomly scattered triangles of arbitrary size
        triangle.color = rand() % 103 + 1;
        VEC4(triangle.vertices[0].position, rand() % 320, rand() % 200, 0);
        VEC4(triangle.vertices[1].position, rand() % 320, rand() % 200, 0);
        VEC4(triangle.vertices[2].position, rand() % 320, rand() % 200, 0);

        // pause rendering when holding down space bar
        if(!kbd_keyDown(KEY_SPACE))
        {
            gfx_drawTriangle(&triangle, &modelViewProj, &buffer);
            utl_printf(&buffer, 1, 1, 15, 0, "Triangles rendered: %d", ++numTris);
            gfx_updateScreen(&buffer);
            delay(50);
        }

    } while(!kbd_keyPressed(KEY_ESC));

    FREE_DRAWBUFFER(buffer);
}
