#ifndef CAMERA_H
#define CAMERA_H

#include "src/math.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Projection camera struct.
 */

typedef struct
{
    mth_Vector4 position;
    mth_Vector4 target;
    mth_Vector4 up;
    mth_Vector4 right;
    mth_Matrix4 view;
    mth_Matrix4 projection;
} gfx_Camera;

#ifdef __cplusplus
}
#endif
#endif
