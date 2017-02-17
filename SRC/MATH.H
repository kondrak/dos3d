#ifndef MATH_H
#define MATH_H

typedef struct
{
    double x;
    double y;
    double z;
} Vector3f;

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
    double dotProduct(const Vector3f *v1, const Vector3f *v2);
    Vector3f crossProduct(const Vector3f *v1, const Vector3f *v2);
    double invLength(const Vector3f *v);
    void   normalize(Vector3f *v);
    void   clamp(double *color);

    // matrix
    void matIdentity(Matrix4f *m);
    void matZero(Matrix4f *m);
    void matOne(Matrix4f *m);
    void matTranspose(Matrix4f *m);
    Vector3f matMulVec(const Matrix4f *m, const Vector3f *v);
    Matrix4f matMul(const Matrix4f *m1, const Matrix4f *m2);
    void matPerspective(Matrix4f *m, const float fov, const float scrRatio, const float nearPlane, const float farPlane);
    void matView(Matrix4f *m, const Vector3f *eye, const Vector3f *target, const Vector3f *up);

    // quaternion
    Quaternion quatConjugate(const Quaternion *q);
    void quatNormalize(Quaternion *q);
    Vector3f quatMulVec(const Quaternion *q, const Vector3f *v);
    Quaternion quatMul(const Quaternion *q1, const Quaternion *q2);

#ifdef __cplusplus
}
#endif
#endif
