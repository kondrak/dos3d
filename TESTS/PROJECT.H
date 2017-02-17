#include "src/graphics.h"
#include "src/math.h"

#include <sys/timeb.h>

// Projection test
void testProjection()
{
    unsigned int elapsed;
    struct timeb startTime, endTime;
    Matrix4f view, projection, vp;
    Vector3f eye, target, up, point1, point2;

    eye.x = -160.f; eye.y = -100.f; eye.z = 0.f;
    target.x = 0.f; target.y = 0.f; target.z = -1.f;
    up.x = 0.f; up.y = 1.f; up.z = 0.f;

    point1.x = 0; point1.y = 0; point1.z = -1.f;
    point2.x = 10; point2.y = 10; point2.z = -1.f;

    matView(&view, &eye, &target, &up);
    matPerspective(&projection, 45, 320.f / 200.f, 0.1f, 5.f);
    vp = matMul(&projection, &view);
    matTranspose(&vp);
    point1 = matMulVec(&vp, &point1);
    point2 = matMulVec(&vp, &point2);
    
    printf("Projection: %f %f %f\r", point1.x, point1.y, point1.z);

    drawLine(point1.x, point1.y, point2.x, point2.y, 3);
    
    ftime(&startTime);
    ftime(&endTime);

    elapsed = (endTime.time - startTime.time)*1000 + endTime.millitm - startTime.millitm;
    printf("Projection: %ums", elapsed);
}
