#ifndef CAMERA_H
#define CAMERA_H

#include "src/math.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    Vector4f position;
    Vector4f target;
    Vector4f up;
    Vector4f right;
    Matrix4f view;
    Matrix4f projection;
} Camera;

#ifdef __cplusplus
}
#endif
#endif
