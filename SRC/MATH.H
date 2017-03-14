#ifndef MATH_H
#define MATH_H

#include <math.h>

#ifndef M_PI
#define M_PI 3.141592654
#endif

typedef struct
{
    double x;
    double y;
    double z;
    double w;
} Vector4f;

typedef struct
{
    double u;
    double v;
} Vector2f;

typedef struct
{
    float m[16];
} Matrix4f;

typedef struct
{
    float x;
    float y;
    float z;
    float w;
} Quaternion;

#ifdef __cplusplus
extern "C" {
#endif

    // vectors
    double dotProduct(const Vector4f *v1, const Vector4f *v2);
    Vector4f crossProduct(const Vector4f *v1, const Vector4f *v2);
    Vector4f vecAdd(const Vector4f *v1, const Vector4f *v2);
    Vector4f vecSub(const Vector4f *v1, const Vector4f *v2);
    Vector4f vecScale(const Vector4f *v, const float scale);
    double invLength(const Vector4f *v);
    void   normalize(Vector4f *v);
    void   clamp(double *color);

    // matrix
    void matIdentity(Matrix4f *m);
    void matZero(Matrix4f *m);
    void matOne(Matrix4f *m);
    void matTranspose(Matrix4f *m);
    Vector4f matMulVec(const Matrix4f *m, const Vector4f *v);
    Matrix4f matMul(const Matrix4f *m1, const Matrix4f *m2);
    void matPerspective(Matrix4f *m, const float fov, const float scrRatio, const float nearPlane, const float farPlane);
    void matView(Matrix4f *m, const Vector4f *eye, const Vector4f *target, const Vector4f *up);

    // quaternion
    Quaternion quatConjugate(const Quaternion *q);
    void quatNormalize(Quaternion *q);
    Vector4f quatMulVec(const Quaternion *q, const Vector4f *v);
    Quaternion quatMul(const Quaternion *q1, const Quaternion *q2);

    // vector/quat
    void rotateVecAxisAngle(Vector4f *v, const float angle, const float x, const float y, const float z);
    void rotateVecQuat(Vector4f *v, const Quaternion *q);
#ifdef __cplusplus
}
#endif
#endif
