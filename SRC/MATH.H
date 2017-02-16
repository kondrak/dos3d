#ifndef MATH_H
#define MATH_H

typedef struct
{
    double m_x;
    double m_y;
    double m_z;
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
    double invLength(const Vector3f *v);
    void   normalize(Vector3f *v);
    void   clamp(double *color);

    // matrix
    void matIdentity(Matrix4f *m);
    void matZero(Matrix4f *m);
    void matOne(Matrix4f *m);
    void matTranspose(Matrix4f *m);
    Vector3f matMulVec(Matrix4f *m, Vector3f *v);
    Matrix4f matMul(Matrix4f *m1, Matrix4f *m2);

    // quaternion
    Quaternion quatConjugate(Quaternion *q);
    void quatNormalize(Quaternion *q);
    Vector3f quatMulVec(Quaternion *q, Vector3f *v);
    Quaternion quatMul(Quaternion *q1, Quaternion *q2);

#ifdef __cplusplus
}
#endif
#endif
