#include "src/bitmap.h"
#include "src/camera.h"
#include "src/graphics.h"
#include <stdlib.h>

// helper function
void setupTextureCube(Vector4f *c);

// Texture mapping test
void testTextureMapping()
{
    unsigned short *keysPressed;
    float t = 0.f;
    int k;
    Vector4f cube[16];
    Triangle triangle[2], screenTriangle[2];
    Camera cam;
    Matrix4f model;
    Matrix4f modelViewProj;
    BITMAP bmp = loadBitmap("brick.bmp");
    unsigned char *buffer = (unsigned char *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT);

    if(!buffer)
    {
        printf("Out of memory!");
        return;
    }

    setPalette(bmp.palette);

    matIdentity(&model);
    matIdentity(&modelViewProj);

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

    // setup world coordinates for pseudo-cube
    //setupTextureCube(&cube);
    triangle[0].color = 3;
    triangle[0].texture = &bmp;
    triangle[0].vertices[0].position.x = 0;
    triangle[0].vertices[0].position.y = 0;
    triangle[0].vertices[0].position.z = 0;
    triangle[0].vertices[0].position.w = 1.f;
    triangle[0].vertices[0].uv.u = 0;
    triangle[0].vertices[0].uv.v = 0;
    
    triangle[0].vertices[1].position.x = 40;
    triangle[0].vertices[1].position.y = 40;
    triangle[0].vertices[1].position.z = 0;
    triangle[0].vertices[1].position.w = 1.f;
    triangle[0].vertices[1].uv.u = 1;
    triangle[0].vertices[1].uv.v = 1;

    triangle[0].vertices[2].position.x = 0;
    triangle[0].vertices[2].position.y = 40;
    triangle[0].vertices[2].position.z = 0;
    triangle[0].vertices[2].position.w = 1.f;
    triangle[0].vertices[2].uv.u = 0;
    triangle[0].vertices[2].uv.v = 1;

    triangle[1].color = 3;
    triangle[1].texture = NULL; //&bmp;
    triangle[1].vertices[0].position.x = 0;
    triangle[1].vertices[0].position.y = 0;
    triangle[1].vertices[0].position.z = 0;
    triangle[1].vertices[0].position.w = 1.f;
    triangle[1].vertices[0].uv.u = 0;
    triangle[1].vertices[0].uv.v = 0;
    
    triangle[1].vertices[1].position.x = 40;
    triangle[1].vertices[1].position.y = 0;
    triangle[1].vertices[1].position.z = 0;
    triangle[1].vertices[1].position.w = 1.f;
    triangle[1].vertices[1].uv.u = 1;
    triangle[1].vertices[1].uv.v = 1;

    triangle[1].vertices[2].position.x = 40;
    triangle[1].vertices[2].position.y = 40;
    triangle[1].vertices[2].position.z = 0;
    triangle[1].vertices[2].position.w = 1.f;
    triangle[1].vertices[2].uv.u = 0;
    triangle[1].vertices[2].uv.v = 1;
    
    for(k = 0; k < 2; k++)
    {
        screenTriangle[k].color = triangle[k].color;
        screenTriangle[k].texture = triangle[k].texture;
        screenTriangle[k].vertices[0].uv = triangle[k].vertices[0].uv;
        screenTriangle[k].vertices[1].uv = triangle[k].vertices[1].uv;
        screenTriangle[k].vertices[2].uv = triangle[k].vertices[2].uv;
    }
    // translate entire cube a bit
    //model.m[12] = -20.f;
    model.m[13] = -20.f;

    keysPressed = translateInput();

    while(!keysPressed[KEY_ESC])
    {
        int i;
        // screen coordinates for the cube we will calculate here
        Vector4f screenCube[16];
        matView(&cam.view, &cam.position, &cam.target, &cam.up);
        matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 5.f);
        modelViewProj = matMul(&cam.view, &cam.projection);
        modelViewProj = matMul(&model, &modelViewProj);

        t += 0.001f;
        model.m[14] = sin(t) * 80.f;

        for(k = 0; k < 2; ++k)
        {
            for(i = 0; i < 3; i++)
            {
                screenTriangle[k].vertices[i].position = matMulVec(&modelViewProj, &triangle[k].vertices[i].position);
                screenTriangle[k].vertices[i].position.x = (screenTriangle[k].vertices[i].position.x * (float)SCREEN_WIDTH)  / (2.0f * screenTriangle[k].vertices[i].position.w) + (SCREEN_WIDTH >> 1);
                screenTriangle[k].vertices[i].position.y = (screenTriangle[k].vertices[i].position.y * (float)SCREEN_HEIGHT) / (2.0f * screenTriangle[k].vertices[i].position.w) + (SCREEN_HEIGHT >> 1);
            }
        }
        /*for(i = 0; i < 16; i++)
      {
      // rotate cube on all axes, just for fun effect and zoom in/out for better perspective test
      rotateVecAxisAngle(&cube[i], 0.001f, 1.f, 1.f, 1.f);
      model.m[14] = sin(t) * 80.f;
      // calculate screen coordinates for transformed square vertices
      screenCube[i] = matMulVec(&modelViewProj, &cube[i]);

      // translate position to screen pixels
      screenCube[i].x = (screenCube[i].x * (float)SCREEN_WIDTH)  / (2.0f * screenCube[i].w) + (SCREEN_WIDTH >> 1);
      screenCube[i].y = (screenCube[i].y * (float)SCREEN_HEIGHT) / (2.0f * screenCube[i].w) + (SCREEN_HEIGHT >> 1);
      }
    */
        clrScrBuffer(buffer);

        drawTriangle(&screenTriangle[0], buffer);
        drawTriangle(&screenTriangle[1], buffer);
        // render the cube walls
        /*for(i = 0; i < 4; i++)
        {
            drawLineVec(&screenCube[4*i + 0], &screenCube[4*i + 1], 3+i, buffer);
            drawLineVec(&screenCube[4*i + 1], &screenCube[4*i + 3], 3+i, buffer);
            drawLineVec(&screenCube[4*i + 0], &screenCube[4*i + 2], 3+i, buffer);
            drawLineVec(&screenCube[4*i + 2], &screenCube[4*i + 3], 3+i, buffer);
        }
    */
        //drawBitmap(&bmp, 0, 0, buffer);
        updateScreen(buffer);

        keysPressed = translateInput();
    }
    
    free(buffer);

    do {
        keysPressed = translateInput();
    } while(keysPressed[KEY_ESC]);
}


void setupTextureCube(Vector4f *c)
{
    int i;
    for(i = 0; i < 4; i++)
    {
        c[i].x = 40*(i%2);
        c[i].y = 40*(i > 1 ? 1 : 0);
        c[i].z = 0.f;
        c[i].w = 1.f;
    }
    
    for(i = 4; i < 8; i++)
    {
        c[i].x = 40;
        c[i].y = 40*(i > 5 ? 1 : 0);
        c[i].z = 40*(i%2);
        c[i].w = 1.f;
    }
    
    for(i = 8; i < 12; i++)
    {
        c[i].x = 40*(i%2);
        c[i].y = 40*(i > 9 ? 1 : 0);
        c[i].z = 40;
        c[i].w = 1.f;
    }
    
    for(i = 12; i < 16; i++)
    {
        c[i].x = 0;
        c[i].y = 40*(i > 13 ? 1 : 0);
        c[i].z = 40*(i%2);
        c[i].w = 1.f;
    }
}
