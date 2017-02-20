#include "src/graphics.h"
#include "src/math.h"

#include <stdlib.h>
#include <sys/timeb.h>

// Projection test
void testProjection(int scrWidth, int scrHeight)
{
    unsigned short *keysPressed;
    unsigned int elapsed, i,x,y;
    struct timeb startTime, endTime;
    Matrix4f view, projection, vp;
    Vector4f eye, target, up;
    Vector4f square[4];
    float eyeX = 0;//-75.f;
    float eyeY = 0;//-25.f;
    float eyeZ = 1.f;
    float eyeRx = 0.f;
    float eyeRy = 0.f;
    unsigned char *buffer = (unsigned char *)malloc(scrWidth*scrHeight);

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
            square[i].z = -8.f; //-8.f * (i%2 ? 1.1 : 1);
            square[i].w = 1.f;
        }

        eye.w = 1.f;
        eye.x = eyeX; eye.y = eyeY; eye.z = eyeZ;
        target.x = eyeRx; target.y = eyeRy; target.z = -1.f; target.w = 1.f;
        up.x = 0.f; up.y = 1.f; up.z = 0.f; up.w = 1.f;

        matView(&view, &eye, &target, &up);
        matPerspective(&projection, 75.f * M_PI /180.f, (float)scrWidth / (float)scrHeight, 0.1f, 5.f);
        vp = matMul(&view, &projection);
        matTranspose(&vp);

        for(i = 0; i < 4; i++)
        {
            square[i] = matMulVec(&vp, &square[i]);
            square[i].x /= square[i].w;
            square[i].y /= square[i].w;
            square[i].z /= square[i].w;

            square[i].x = (square[i].x * (float)scrWidth)  / (2.0f * square[i].w) + (scrWidth >> 1);
            square[i].y = (square[i].y * (float)scrHeight) / (2.0f * square[i].w) + (scrHeight >> 1);
            //printf("%.2f %.2f %.2f %.2f\n", square[i].x, square[i].y, square[i].z, square[i].w);
        }

        clrScrBuffer(buffer);

        drawLine(square[0].x, square[0].y, square[1].x, square[1].y, 3, buffer);
        drawLine(square[1].x, square[1].y, square[3].x, square[3].y, 3, buffer);
        drawLine(square[0].x, square[0].y, square[2].x, square[2].y, 3, buffer);
        drawLine(square[2].x, square[2].y, square[3].x, square[3].y, 3, buffer);

        updateScreen(buffer);

        keysPressed = translateInput();

        if(keysPressed[KEY_W])
            eyeZ -= 0.01f;

        if(keysPressed[KEY_S])
            eyeZ += 0.01f;

        if(keysPressed[KEY_A])
            eyeX -= 0.1f;

        if(keysPressed[KEY_D])
            eyeX += 0.1f;

        if(keysPressed[KEY_LEFT])
            eyeRx += 0.0001f;

        if(keysPressed[KEY_RIGHT])
            eyeRx -= 0.0001f;

        if(keysPressed[KEY_PGUP])
            eyeRy -= 0.0001f;

        if(keysPressed[KEY_PGDN])
            eyeRy += 0.0001f;
    }
    
    ftime(&startTime);
    ftime(&endTime);

    free(buffer);

    elapsed = (endTime.time - startTime.time)*1000 + endTime.millitm - startTime.millitm;

    do {
        keysPressed = translateInput();
    } while(keysPressed[KEY_ESC]);
}
