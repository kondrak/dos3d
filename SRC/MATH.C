#include "src/math.h"

// quick inverse sqrt()
double qInvSqrt(double number)
{
    long i;
    float x2, y;

    x2 = number * 0.5F;
    y = number;
    i = *(long *)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float *)&i;
    y = y * (1.5f - (x2 * y * y));   // 1st iteration
    y = y * (1.5f - (x2 * y * y));   // 2nd iteration

    return y;
}

// vector dot product
double dotProduct(const Vector3f *v1, const Vector3f *v2)
{
    return v1->m_x * v2->m_x + v1->m_y * v2->m_y + v1->m_z * v2->m_z;
}

// inverse vector length
double invLength(const Vector3f *v)
{
    return qInvSqrt(v->m_x*v->m_x + v->m_y*v->m_y + v->m_z*v->m_z);
}

// normalize a vector
void normalize(Vector3f *v)
{
    double l = qInvSqrt(v->m_x*v->m_x + v->m_y*v->m_y + v->m_z*v->m_z);

    v->m_x *= l;
    v->m_y *= l;
    v->m_z *= l;
}

// clamp RGB value to [0-255]
void clamp(double *color)
{
    if (color[0] > 0xFF) color[0] = 0xFF;
    if (color[1] > 0xFF) color[1] = 0xFF;
    if (color[2] > 0xFF) color[2] = 0xFF;

    if (color[0] < 0x00) color[0] = 0x00;
    if (color[1] < 0x00) color[1] = 0x00;
    if (color[2] < 0x00) color[2] = 0x00;
}

void matIdentity(Matrix4f *m)
{
    m->m[0]  = m->m[5]  = m->m[10] = m->m[15] = 1.f;
    m->m[1]  = m->m[2]  = m->m[3]  = m->m[4]  = 0.f;
    m->m[6]  = m->m[7]  = m->m[8]  = m->m[9]  = 0.f;
    m->m[11] = m->m[12] = m->m[13] = m->m[14] = 0.f;
}

void matZero(Matrix4f *m)
{
    int i;
    for(i = 0; i < 16; ++i)
        m->m[i] = 0.f;
}

void matOne(Matrix4f *m)
{
    int i;
    for(i = 0; i < 16; ++i)
        m->m[i] = 1.f;
}


void matTranspose(Matrix4f *m)
{
    int i, j;
    Matrix4f temp;

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

Vector3f matMulVec(Matrix4f *m, Vector3f *v)
{
    Vector3f r;
    r.m_x = m->m[0] * v->m_x + m->m[1] * v->m_y + m->m[2]  * v->m_z + m->m[3]  * 1.f;
    r.m_y = m->m[4] * v->m_x + m->m[5] * v->m_y + m->m[6]  * v->m_z + m->m[7]  * 1.f;
    r.m_z = m->m[8] * v->m_x + m->m[9] * v->m_y + m->m[10] * v->m_z + m->m[11] * 1.f;

    return r;
}

Matrix4f matMul(Matrix4f *m1, Matrix4f *m2)
{
    Matrix4f r;
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

Quaternion quatConjugate(Quaternion *q)
{
    Quaternion r;
    r.x = -q->x;
    r.y = -q->y;
    r.z = -q->z;
    r.w = q->w;

    return r;
}

void quatNormalize(Quaternion *q)
{
    double l = qInvSqrt(q->x*q->x + q->y*q->y + q->z*q->z + q->w*q->w);
    q->x *= l;
    q->y *= l;
    q->z *= l;
    q->w *= l;
}

Vector3f quatMulVec(Quaternion *q, Vector3f *v)
{
    Vector3f vn, r;
    Quaternion vq, cq, rq, rq2;
    vn.m_x = v->m_x;
    vn.m_y = v->m_y;
    vn.m_z = v->m_z;
    normalize(&vn);

    vq.x = vn.m_x;
    vq.y = vn.m_y;
    vq.z = vn.m_z;
    vq.w = 0.f;

    cq = quatConjugate(q);
    rq = quatMul(&vq, &cq);
    rq2 = quatMul(q, &rq);

    r.m_x = rq2.x;
    r.m_y = rq2.y;
    r.m_z = rq2.z;

    return r;
}

Quaternion quatMul(Quaternion *q1, Quaternion *q2)
{
    Quaternion r;
    r.x = q1->w*q2->x + q1->x*q2->w + q1->y*q2->z - q1->z*q2->y;
    r.y = q1->w*q2->y - q1->x*q2->z + q1->y*q2->w + q1->z*q2->x;
    r.z = q1->w*q2->z + q1->x*q2->y - q1->y*q2->x + q1->z*q2->w;
    r.w = q1->w*q2->w - q1->x*q2->x - q1->y*q2->y - q1->z*q2->z;

    return r;
}
