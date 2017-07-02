#include "src/math.h"
#include <math.h>
#include <stdint.h>

// internal: quick inverse sqrt()
static double qInvSqrt(double number)
{
    int32_t i;
    float x2, y;

    x2 = number * 0.5F;
    y = number;
    i = *(int32_t *)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float *)&i;
    y = y * (1.5f - (x2 * y * y));   // 1st iteration
    y = y * (1.5f - (x2 * y * y));   // 2nd iteration

    return y;
}

/* ***** */
mth_Vector4 mth_crossProduct(const mth_Vector4 *v1, const mth_Vector4 *v2)
{
    mth_Vector4 r;
    r.x = v1->y*v2->z - v1->z*v2->y;
    r.y = v1->z*v2->x - v1->x*v2->z;
    r.z = v1->x*v2->y - v1->y*v2->x;
    r.w = 1.f;

    return r;
}

/* ***** */
mth_Vector4 mth_vecAdd(const mth_Vector4 *v1, const mth_Vector4 *v2)
{
    mth_Vector4 r;
    r.x = v1->x + v2->x;
    r.y = v1->y + v2->y;
    r.z = v1->z + v2->z;
    r.w = 1.f;
    return r;
}

/* ***** */
mth_Vector4 mth_vecSub(const mth_Vector4 *v1, const mth_Vector4 *v2)
{
    mth_Vector4 r;
    r.x = v1->x - v2->x;
    r.y = v1->y - v2->y;
    r.z = v1->z - v2->z;
    r.w = 1.f;
    return r;
}

/* ***** */
mth_Vector4 mth_vecScale(const mth_Vector4 *v, const float scale)
{
    mth_Vector4 r;
    r.x = v->x * scale;
    r.y = v->y * scale;
    r.z = v->z * scale;
    r.w = 1.f;
    return r;
}

/* ***** */
double mth_dotProduct(const mth_Vector4 *v1, const mth_Vector4 *v2)
{
    return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
}

/* ***** */
double mth_lengthSquare(const mth_Vector4 *v)
{
    return (v->x*v->x + v->y*v->y + v->z*v->z);
}

/* ***** */
double mth_invLength(const mth_Vector4 *v)
{
    return qInvSqrt(v->x*v->x + v->y*v->y + v->z*v->z);
}

/* ***** */
void mth_normalize(mth_Vector4 *v)
{
    double l = qInvSqrt(v->x*v->x + v->y*v->y + v->z*v->z);

    v->x *= l;
    v->y *= l;
    v->z *= l;
}

/* ***** */
mth_Vector4 mth_matMulVec(const mth_Matrix4 *m, const mth_Vector4 *v)
{
    mth_Vector4 r;
    r.x = m->m[0] * v->x + m->m[4] * v->y + m->m[8]  * v->z + m->m[12] * v->w;
    r.y = m->m[1] * v->x + m->m[5] * v->y + m->m[9]  * v->z + m->m[13] * v->w;
    r.z = m->m[2] * v->x + m->m[6] * v->y + m->m[10] * v->z + m->m[14] * v->w;
    r.w = m->m[3] * v->x + m->m[7] * v->y + m->m[11] * v->z + m->m[15] * v->w;

    return r;
}

/* ***** */
mth_Matrix4 mth_matMul(const mth_Matrix4 *m1, const mth_Matrix4 *m2)
{
    mth_Matrix4 r;
    r.m[0] = m1->m[0] * m2->m[0] + m1->m[1] * m2->m[4] + m1->m[2] * m2->m[8]  + m1->m[3] * m2->m[12];
    r.m[1] = m1->m[0] * m2->m[1] + m1->m[1] * m2->m[5] + m1->m[2] * m2->m[9]  + m1->m[3] * m2->m[13];
    r.m[2] = m1->m[0] * m2->m[2] + m1->m[1] * m2->m[6] + m1->m[2] * m2->m[10] + m1->m[3] * m2->m[14];
    r.m[3] = m1->m[0] * m2->m[3] + m1->m[1] * m2->m[7] + m1->m[2] * m2->m[11] + m1->m[3] * m2->m[15];

    r.m[4] = m1->m[4] * m2->m[0] + m1->m[5] * m2->m[4] + m1->m[6] * m2->m[8]  + m1->m[7] * m2->m[12];
    r.m[5] = m1->m[4] * m2->m[1] + m1->m[5] * m2->m[5] + m1->m[6] * m2->m[9]  + m1->m[7] * m2->m[13];
    r.m[6] = m1->m[4] * m2->m[2] + m1->m[5] * m2->m[6] + m1->m[6] * m2->m[10] + m1->m[7] * m2->m[14];
    r.m[7] = m1->m[4] * m2->m[3] + m1->m[5] * m2->m[7] + m1->m[6] * m2->m[11] + m1->m[7] * m2->m[15];

    r.m[8]  = m1->m[8] * m2->m[0] + m1->m[9] * m2->m[4] + m1->m[10] * m2->m[8]  + m1->m[11] * m2->m[12];
    r.m[9]  = m1->m[8] * m2->m[1] + m1->m[9] * m2->m[5] + m1->m[10] * m2->m[9]  + m1->m[11] * m2->m[13];
    r.m[10] = m1->m[8] * m2->m[2] + m1->m[9] * m2->m[6] + m1->m[10] * m2->m[10] + m1->m[11] * m2->m[14];
    r.m[11] = m1->m[8] * m2->m[3] + m1->m[9] * m2->m[7] + m1->m[10] * m2->m[11] + m1->m[11] * m2->m[15];

    r.m[12] = m1->m[12] * m2->m[0] + m1->m[13] * m2->m[4] + m1->m[14] * m2->m[8]  + m1->m[15] * m2->m[12];
    r.m[13] = m1->m[12] * m2->m[1] + m1->m[13] * m2->m[5] + m1->m[14] * m2->m[9]  + m1->m[15] * m2->m[13];
    r.m[14] = m1->m[12] * m2->m[2] + m1->m[13] * m2->m[6] + m1->m[14] * m2->m[10] + m1->m[15] * m2->m[14];
    r.m[15] = m1->m[12] * m2->m[3] + m1->m[13] * m2->m[7] + m1->m[14] * m2->m[11] + m1->m[15] * m2->m[15];

    return r;
}

/* ***** */
void mth_matIdentity(mth_Matrix4 *m)
{
    m->m[0]  = m->m[5]  = m->m[10] = m->m[15] = 1.f;
    m->m[1]  = m->m[2]  = m->m[3]  = m->m[4]  = 0.f;
    m->m[6]  = m->m[7]  = m->m[8]  = m->m[9]  = 0.f;
    m->m[11] = m->m[12] = m->m[13] = m->m[14] = 0.f;
}

/* ***** */
void mth_matTranspose(mth_Matrix4 *m)
{
    int i, j;
    mth_Matrix4 temp;

    for (i = 0; i < 16; i++)
        temp.m[i] = m->m[i];

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            m->m[i * 4 + j] = temp.m[i + j * 4];
        }
    }
}

/* ***** */
void mth_matPerspective(mth_Matrix4 *m, const float fov, const float scrRatio, const float nearPlane, const float farPlane)
{
    int i;
    float tanFov;
    // zero the matrix first
    for(i = 0; i < 16; ++i)
        m->m[i] = 0.f;

    tanFov = tan(0.5f * fov);
    m->m[0]  = 1.f / (scrRatio * tanFov);
    m->m[5]  = 1.f / tanFov;
    m->m[10] = -(farPlane + nearPlane) / (farPlane - nearPlane);
    m->m[11] = -1.f;
    m->m[14] = -2.f * farPlane * nearPlane / (farPlane - nearPlane);
}

/* ***** */
void mth_matOrtho(mth_Matrix4 *m, const float left, const float right, const float bottom, const float top, const float nearPlane, const float farPlane)
{
    mth_matIdentity(m);

    m->m[0]  = 2.f / (right - left);
    m->m[5]  = 2.f / (top - bottom);
    m->m[10] = -2.f / (farPlane - nearPlane);
    m->m[12] = -(right + left) / (right - left);
    m->m[13] = -(top + bottom) / (top - bottom);
    m->m[14] = -(farPlane + nearPlane) / (farPlane - nearPlane);
}

/* ***** */
void mth_matView(mth_Matrix4 *m, const mth_Vector4 *eye, const mth_Vector4 *target, const mth_Vector4 *up)
{
    mth_Vector4 x,y,z;

    z.x = target->x;
    z.y = target->y;
    z.z = target->z;
    z.w = target->w;
    mth_normalize(&z);
    x = mth_crossProduct(&z, up);
    mth_normalize(&x);
    y = mth_crossProduct(&x, &z);

    mth_matIdentity(m);

    m->m[0] = x.x;
    m->m[4] = x.y;
    m->m[8] = x.z;

    m->m[1] = y.x;
    m->m[5] = y.y;
    m->m[9] = y.z;

    m->m[2] = -z.x;
    m->m[6] = -z.y;
    m->m[10] = -z.z;

    m->m[12] = -mth_dotProduct(&x, eye);
    m->m[13] = -mth_dotProduct(&y, eye);
    m->m[14] = mth_dotProduct(&z, eye);
}

/* ***** */
mth_Quaternion mth_quatMul(const mth_Quaternion *q1, const mth_Quaternion *q2)
{
    mth_Quaternion r;
    r.x = q1->w*q2->x + q1->x*q2->w + q1->y*q2->z - q1->z*q2->y;
    r.y = q1->w*q2->y - q1->x*q2->z + q1->y*q2->w + q1->z*q2->x;
    r.z = q1->w*q2->z + q1->x*q2->y - q1->y*q2->x + q1->z*q2->w;
    r.w = q1->w*q2->w - q1->x*q2->x - q1->y*q2->y - q1->z*q2->z;

    return r;
}

/* ***** */
mth_Quaternion mth_quatConjugate(const mth_Quaternion *q)
{
    mth_Quaternion r;
    r.x = -q->x;
    r.y = -q->y;
    r.z = -q->z;
    r.w = q->w;

    return r;
}

/* ***** */
mth_Vector4 mth_quatMulVec(const mth_Quaternion *q, const mth_Vector4 *v)
{
    mth_Vector4 vn, r;
    mth_Quaternion vq, cq, rq, rq2;
    vn.x = v->x;
    vn.y = v->y;
    vn.z = v->z;
    mth_normalize(&vn);

    vq.x = vn.x;
    vq.y = vn.y;
    vq.z = vn.z;
    vq.w = 0.f;

    cq = mth_quatConjugate(q);
    rq = mth_quatMul(&vq, &cq);
    rq2 = mth_quatMul(q, &rq);

    r.x = rq2.x;
    r.y = rq2.y;
    r.z = rq2.z;

    return r;
}

/* ***** */
void mth_quatNormalize(mth_Quaternion *q)
{
    double l = qInvSqrt(q->x*q->x + q->y*q->y + q->z*q->z + q->w*q->w);
    q->x *= l;
    q->y *= l;
    q->z *= l;
    q->w *= l;
}

/* ***** */
void mth_rotateVecAxisAngle(mth_Vector4 *v, const float angle, const float x, const float y, const float z)
{
    mth_Quaternion q;
    float hAngle = angle/2.f;
    q.x = x * sin(hAngle);
    q.y = y * sin(hAngle);
    q.z = z * sin(hAngle);
    q.w = cos(hAngle);

    mth_rotateVecQuat(v, &q);
}

/* ***** */
void mth_rotateVecQuat(mth_Vector4 *v, const mth_Quaternion *q)
{
    mth_Quaternion r, qv, qc, viewQuat;

    viewQuat.x = v->x;
    viewQuat.y = v->y;
    viewQuat.z = v->z;
    viewQuat.w = 0.f;

    qc = mth_quatConjugate(q);
    qv = mth_quatMul(q, &viewQuat);
    r  = mth_quatMul(&qv, &qc);

    v->x = r.x;
    v->y = r.y;
    v->z = r.z;
}
