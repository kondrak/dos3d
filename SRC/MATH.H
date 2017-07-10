#ifndef MATH_H
#define MATH_H

#include <math.h>

/*
 * Common math functions and structures.
 */

#ifndef M_PI
#define M_PI 3.141592654
#endif

#ifdef __cplusplus
extern "C" {
#endif

    // 4D vector
    typedef struct
    {
        double x;
        double y;
        double z;
        double w;
    } mth_Vector4;

    // 2D vector
    typedef struct
    {
        double u;
        double v;
    } mth_Vector2;

    // 4x4 matrix
    typedef struct
    {
        double m[16];
    } mth_Matrix4;

    // quaternion
    typedef struct
    {
        double x;
        double y;
        double z;
        double w;
    } mth_Quaternion;

    // helper macro to reduce boilerplate when initializing a mth_Vector4
    #define VEC4(vec, vx, vy, vz) { vec.x = vx; vec.y = vy; vec.z = vz; vec.w = 1.0; }

    /* *** Interface *** */

    /*
     * vector operations
     */

    // v1 x v2 cross product
    mth_Vector4 mth_crossProduct(const mth_Vector4 *v1, const mth_Vector4 *v2);

    // v1 + v2
    mth_Vector4 mth_vecAdd(const mth_Vector4 *v1, const mth_Vector4 *v2);

    // v1 - v2
    mth_Vector4 mth_vecSub(const mth_Vector4 *v1, const mth_Vector4 *v2);

    // scale vector
    mth_Vector4 mth_vecScale(const mth_Vector4 *v, const float scale);

    // v1 * v2 dot product
    double mth_dotProduct(const mth_Vector4 *v1, const mth_Vector4 *v2);

    // return lenght squared of vector v
    double mth_lengthSquare(const mth_Vector4 *v);

    // return inverse length of vector v
    double mth_invLength(const mth_Vector4 *v);

    // normalize vector v
    void mth_normalize(mth_Vector4 *v);

    /*
     * matrix operations
     */

    // m * v multiplication
    mth_Vector4 mth_matMulVec(const mth_Matrix4 *m, const mth_Vector4 *v);

    // m1 * m2 multiplication
    mth_Matrix4 mth_matMul(const mth_Matrix4 *m1, const mth_Matrix4 *m2);

    // set matrix m to identity
    void mth_matIdentity(mth_Matrix4 *m);

    // transpose matrix m
    void mth_matTranspose(mth_Matrix4 *m);

    // turn matrix m into perspective projection matrix
    void mth_matPerspective(mth_Matrix4 *m, const float fov, const float scrRatio, const float nearPlane, const float farPlane);

    // turn matrix m into orthographic projection matrix
    void mth_matOrtho(mth_Matrix4 *m, const float left, const float right, const float bottom, const float top, const float nearPlane, const float farPlane);

    // turn matrix m into view matrix
    void mth_matView(mth_Matrix4 *m, const mth_Vector4 *eye, const mth_Vector4 *target, const mth_Vector4 *up);

    /*
     * quaternion operations
     */

    // q1 * q2 multiplication
    mth_Quaternion mth_quatMul(const mth_Quaternion *q1, const mth_Quaternion *q2);

    // return conjugate of quaternion q
    mth_Quaternion mth_quatConjugate(const mth_Quaternion *q);

    // q * v multiplication
    mth_Vector4 mth_quatMulVec(const mth_Quaternion *q, const mth_Vector4 *v);

    // normalize quaternion q
    void mth_quatNormalize(mth_Quaternion *q);

    /*
     * vector<->quaternion operations
     */

    // rotate vector using axis and angle
    void mth_rotateVecAxisAngle(mth_Vector4 *v, const float angle, const float x, const float y, const float z);

    // rotate vector using a quaternion
    void mth_rotateVecQuat(mth_Vector4 *v, const mth_Quaternion *q);

#ifdef __cplusplus
}
#endif
#endif
