#include "src/graphics.h"
#include "src/math.h"

#include <sys/timeb.h>

// Projection test
void testProjection()
{
    unsigned int elapsed, i;
    struct timeb startTime, endTime;
    Matrix4f view, projection, vp;
    Vector3f eye, target, up;
    Vector3f square[4];

    for(i = 0; i < 4; i++)
    {
        square[i].x = 50*(i%2);
        square[i].y = 50*(i > 1 ? 1 : 0);
        square[i].z = -1.f;
    }

    eye.x = -135.f; eye.y = -75.f; eye.z = 0.f;
    target.x = 0.f; target.y = 0.f; target.z = -1.f;
    up.x = 0.f; up.y = 1.f; up.z = 0.f;

    matView(&view, &eye, &target, &up);
    matPerspective(&projection, 45, 320.f / 200.f, 0.1f, 5.f);
    vp = matMul(&projection, &view);
    matTranspose(&vp);

    for(i = 0; i < 4; i++)
    {
        square[i] = matMulVec(&vp, &square[i]);
        printf("%.2f %.2f %.2f\n", square[i].x, square[i].y, square[i].z);
    }


    drawLine(square[0].x, square[0].y, square[1].x, square[1].y, 3);
    drawLine(square[1].x, square[1].y, square[3].x, square[3].y, 3);
    drawLine(square[0].x, square[0].y, square[2].x, square[2].y, 3);
    drawLine(square[2].x, square[2].y, square[3].x, square[3].y, 3);
    
    ftime(&startTime);
    ftime(&endTime);

    elapsed = (endTime.time - startTime.time)*1000 + endTime.millitm - startTime.millitm;
    //printf("Projection: %ums", elapsed);
}
