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
