#include "src/camera.h"
#include "src/triangle.h"
#include <dos.h>
#include <stdlib.h>

// Triangle rendering test
void testTriangles()
{
    unsigned short *keysPressed;
    Camera cam;
    int i, paused = 0, numTris = 0;
    Triangle triangle, screenTriangle;
    Matrix4f model;
    Matrix4f modelViewProj;
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

    // translate entire cube a bit
    model.m[12] = -160.f;
    model.m[13] = -100.f;

    keysPressed = translateInput();
    clrScrBufferColor(buffer, 105);

    while(!keysPressed[KEY_ESC])
    {
        triangle.color = rand() % 103 + 1;
        triangle.texture = NULL;
        triangle.vertices[0].position.x = rand() % 320;
        triangle.vertices[0].position.y = rand() % 200;
        triangle.vertices[0].position.z = 0;
        triangle.vertices[0].position.w = 1.f;

        triangle.vertices[1].position.x = rand() % 320;
        triangle.vertices[1].position.y = rand() % 200;
        triangle.vertices[1].position.z = 0;
        triangle.vertices[1].position.w = 1.f;

        triangle.vertices[2].position.x = rand() % 320;
        triangle.vertices[2].position.y = rand() % 200;
        triangle.vertices[2].position.z = 0;
        triangle.vertices[2].position.w = 1.f;

        screenTriangle.color = triangle.color;
        screenTriangle.texture = NULL;
        
        matView(&cam.view, &cam.position, &cam.target, &cam.up);
        matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 5.f);
        modelViewProj = matMul(&cam.view, &cam.projection);
        modelViewProj = matMul(&model, &modelViewProj);

        for(i = 0; i < 3; i++)
        {
            screenTriangle.vertices[i].position = matMulVec(&modelViewProj, &triangle.vertices[i].position);
            screenTriangle.vertices[i].position.x = (screenTriangle.vertices[i].position.x * (float)SCREEN_WIDTH)  / (2.0f * screenTriangle.vertices[i].position.w) + (SCREEN_WIDTH >> 1);
            screenTriangle.vertices[i].position.y = (screenTriangle.vertices[i].position.y * (float)SCREEN_HEIGHT) / (2.0f * screenTriangle.vertices[i].position.w) + (SCREEN_HEIGHT >> 1);
        }

        if(paused == 0)
        {
            drawTriangle(&screenTriangle, buffer);
            updateScreen(buffer);
            fprintf(stdout, "Triangles rendered: %d\r", ++numTris);
            fflush(stdout);
            delay(50);
        }

        keysPressed = translateInput();

        // pause triangle rendering when holding down space bar
        paused = keysPressed[KEY_SPACE];
    }

    free(buffer);

    do {
        keysPressed = translateInput();
    } while(keysPressed[KEY_ESC]);
}
