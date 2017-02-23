#include "src/graphics.h"
#include "src/math.h"

#include <stdlib.h>
#include <sys/timeb.h>

typedef struct
{
    Vector4f position;
    Vector4f target;
    Vector4f up;
    Vector4f right;
    Matrix4f view;
    Matrix4f projection;
} Camera;

// Projection test
void testProjection(int scrWidth, int scrHeight)
{
    unsigned short *keysPressed;
    unsigned int elapsed, i,x,y;
    struct timeb startTime, endTime;
    Vector4f vs, rs;
    Vector4f square[4];
    Camera cam;
    Matrix4f vp;
    unsigned char *buffer = (unsigned char *)malloc(scrWidth*scrHeight);

    cam.position.x = 0;
    cam.position.y = 0;
    cam.position.z = 1.f;
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

    if(!buffer)
    {
        printf("Out of memory!");
        return;
    }

    keysPressed = translateInput();

    while(!keysPressed[KEY_ESC])
    {
        for(i = 0; i < 4; i++)
        {
            square[i].x = 0 + 50*(i%2);
            square[i].y = 0 + 50*(i > 1 ? 1 : 0);
            square[i].z = -80.f;
            square[i].w = 1.f;
        }

        matView(&cam.view, &cam.position, &cam.target, &cam.up);
        matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)scrWidth / (float)scrHeight, 0.1f, 5.f);
        vp = matMul(&cam.view, &cam.projection);
        matTranspose(&vp);

        for(i = 0; i < 4; i++)
        {
            square[i] = matMulVec(&vp, &square[i]);

            // translate position to screen pixels
            square[i].x = (square[i].x * (float)scrWidth)  / (2.0f * square[i].w) + (scrWidth >> 1);
            square[i].y = (square[i].y * (float)scrHeight) / (2.0f * square[i].w) + (scrHeight >> 1);
        }

        clrScrBuffer(buffer);

        drawLine(square[0].x, square[0].y, square[1].x, square[1].y, 3, buffer);
        drawLine(square[1].x, square[1].y, square[3].x, square[3].y, 3, buffer);
        drawLine(square[0].x, square[0].y, square[2].x, square[2].y, 3, buffer);
        drawLine(square[2].x, square[2].y, square[3].x, square[3].y, 3, buffer);

        updateScreen(buffer);

        keysPressed = translateInput();

        vs = vecScale(&cam.target, 0.1f);
        rs = vecScale(&cam.right, 0.1f);

        if(keysPressed[KEY_W])
            cam.position = vecAdd(&cam.position, &vs);

        if(keysPressed[KEY_S])
            cam.position = vecSub(&cam.position, &vs);

        if(keysPressed[KEY_A])
            cam.position = vecSub(&cam.position, &rs);

        if(keysPressed[KEY_D])
            cam.position = vecAdd(&cam.position, &rs);

        if(keysPressed[KEY_LEFT])
        {
            rotateVecAxisAngle(&cam.target, 0.001f, cam.up.x, cam.up.y, cam.up.z);
            cam.right = crossProduct(&cam.target, &cam.up);
        }

        if(keysPressed[KEY_RIGHT])
        {
            rotateVecAxisAngle(&cam.target, -0.001f, cam.up.x, cam.up.y, cam.up.z);
            cam.right = crossProduct(&cam.target, &cam.up);
        }

        if(keysPressed[KEY_PGUP])
        {
            rotateVecAxisAngle(&cam.target, -0.001f, cam.right.x, cam.right.y, cam.right.z);
            cam.right = crossProduct(&cam.target, &cam.up);
        }

        if(keysPressed[KEY_PGDN])
        {
            rotateVecAxisAngle(&cam.target, 0.001f, cam.right.x, cam.right.y, cam.right.z);
            cam.up = crossProduct(&cam.right, &cam.target);
        }
    }
    
    ftime(&startTime);
    ftime(&endTime);

    free(buffer);

    elapsed = (endTime.time - startTime.time)*1000 + endTime.millitm - startTime.millitm;

    do {
        keysPressed = translateInput();
    } while(keysPressed[KEY_ESC]);
}
