#include "src/triangle.h"
#include "src/utils.h"
#define VERTEX_SWAP(v1, v2) { gfx_Vertex s = v2; v2 = v1; v1 = s; }

// simplest case: will plot either a flat bottom or flat top triangles
enum TriangleType
{
    FLAT_BOTTOM,
    FLAT_TOP
};

// internal: perform triangle rendering based on its type
static void drawTriangleType(const gfx_Triangle *t, const gfx_Vertex *v0, const gfx_Vertex *v1, const gfx_Vertex *v2, unsigned char *buffer, enum TriangleType type, short colorKey, enum TextureMapping tm);
static void perspectiveTextureMap(const gfx_Triangle *t, const gfx_Vertex *v0, const gfx_Vertex *v1, const gfx_Vertex *v2, unsigned char *buffer, enum TriangleType type, double dxLeft, double dxRight, double yDir, short colorKey);
static void affineTextureMap(const gfx_Triangle *t, const gfx_Vertex *v0, const gfx_Vertex *v1, const gfx_Vertex *v2, unsigned char *buffer, enum TriangleType type, double dxLeft, double dxRight, double yDir, double invDy, short colorKey);

void gfx_drawTriangle(const gfx_Triangle *t, unsigned char *buffer)
{
    gfx_drawTriangleColorKey(t, buffer, -1);
}

/* ***** */
void gfx_drawTriangleTexMap(const gfx_Triangle *t, unsigned char *buffer, enum TextureMapping tm)
{
    // draw triangle to screen buffer without any color keying
    gfx_drawTriangleColorKeyTexMap(t, buffer, -1, tm);
}

void gfx_drawTriangleColorKey(const gfx_Triangle *t, unsigned char *buffer, short colorKey)
{
    gfx_drawTriangleColorKeyTexMap(t, buffer, colorKey, AFFINE);
}

/* ***** */
void gfx_drawTriangleColorKeyTexMap(const gfx_Triangle *t, unsigned char *buffer, short colorKey, enum TextureMapping tm)
{
    gfx_Vertex v0, v1, v2;

    v0 = t->vertices[0];
    v1 = t->vertices[1];
    v2 = t->vertices[2];

    // sort vertices so that v0 is topmost, then v2, then v1
    if(v2.position.y > v1.position.y)
    {
        v2 = t->vertices[1];
        v1 = t->vertices[2];
    }

    if(v0.position.y > v1.position.y)
    {
        v0 = v1;
        v1 = t->vertices[0];
    }

    if(v0.position.y > v2.position.y)
        VERTEX_SWAP(v0, v2)

    // handle 2 basic cases of flat bottom and flat top triangles
    if(v1.position.y == v2.position.y)
        drawTriangleType(t, &v0, &v1, &v2, buffer, FLAT_BOTTOM, colorKey, tm);
    else if(v0.position.y == v1.position.y)
        drawTriangleType(t, &v2, &v1, &v0, buffer, FLAT_TOP, colorKey, tm);
    else
    {
        // "Non-trivial" triangles will be broken down into a composition of flat bottom and flat top triangles.
        // For this, we need to calculate a new vertex, v3 and interpolate its UV coordinates.
        gfx_Vertex v3;
        mth_Vector4 diff, diff2;
        double ratioU = 1, ratioV = 1;

        v3.position.x = v0.position.x + ((float)(v2.position.y - v0.position.y) / (float)(v1.position.y - v0.position.y)) * (v1.position.x - v0.position.x);
        v3.position.y = v2.position.y;

        diff  = mth_vecSub(&v1.position, &v0.position);
        diff2 = mth_vecSub(&v3.position, &v0.position);

        if(diff.x != 0)
            ratioU = diff2.x / diff.x;

        if(diff.y != 0)
            ratioV = diff2.y / diff.y;

        // lerp Z and UV for v3. For perspective texture mapping calculate u/z, v/z, for affine skip unnecessary divisions
        if(tm == PERSPECTIVE)
        {
            float invV0Z = 1.f/v0.position.z;
            float invV1Z = 1.f/v1.position.z;

            // get the z value for v3 by interpolating 1/z, since that can be done using lerp (at this point we skip v3.w - it won't be relevant anymore)
            if((v0.position.x - v1.position.x) != 0.0)
                v3.position.z = 1.0 / LERP(invV1Z, invV0Z, (v3.position.x - v1.position.x) / (v0.position.x - v1.position.x));
            else
                v3.position.z = v0.position.z;

            v3.uv.u = v3.position.z * LERP(v0.uv.u*invV0Z, v1.uv.u*invV1Z, ratioU);
            v3.uv.v = v3.position.z * LERP(v0.uv.v*invV0Z, v1.uv.v*invV1Z, ratioV);
        }
        else
        {
            // simple Intercept Theorem is fine in case of affine texture mapping (skip v3.w again)
            v3.position.z = v0.position.z + ((float)(v2.position.y - v0.position.y) / (float)(v1.position.y - v0.position.y)) * (v1.position.z - v0.position.z);
            v3.uv.u = LERP(v0.uv.u, v1.uv.u, ratioU);
            v3.uv.v = LERP(v0.uv.v, v1.uv.v, ratioV);
        }

        // this swap is done to maintain consistent renderer behavior
        if(v3.position.x < v2.position.x)
            VERTEX_SWAP(v3, v2)

        // draw the composition of both triangles to form the desired shape
        drawTriangleType(t, &v0, &v3, &v2, buffer, FLAT_BOTTOM, colorKey, tm);
        drawTriangleType(t, &v1, &v3, &v2, buffer, FLAT_TOP, colorKey, tm);
    }
}


/*
 * Depending on the triangle type, the order of processed vertices is as follows:
 *
 * v0         v0----v1
 * |\         |     /
 * | \        |    /
 * |  \       |   /
 * |   \      |  /
 * |    \     | /
 * |     \    |/
 * v2-----v1  v2
 */
static void drawTriangleType(const gfx_Triangle *t, const gfx_Vertex *v0, const gfx_Vertex *v1, const gfx_Vertex *v2, unsigned char *buffer, enum TriangleType type, const short colorKey, enum TextureMapping tm)
{
    double invDy, dxLeft, dxRight, xLeft, xRight;
    double y, yDir = 1;

    if(type == FLAT_BOTTOM)
    {
        invDy  = 1.f / (v2->position.y - v0->position.y);
    }
    else
    {
        invDy  = 1.f / (v0->position.y - v2->position.y);
        yDir = -1;
    }
    dxLeft  = (v2->position.x - v0->position.x) * invDy;
    dxRight = (v1->position.x - v0->position.x) * invDy;
    xLeft  = v0->position.x;
    xRight = xLeft;

    if(!t->texture)
    {
        for(y = v0->position.y; ; y += yDir)
        {
            if(type == FLAT_TOP && y < v2->position.y)
            {
                break;
            }
            else if(type == FLAT_BOTTOM && y > v2->position.y)
            {
                // to avoid pixel wide gaps, render extra line at the junction between two final points
                gfx_drawLine(xLeft-dxLeft, y, xRight-dxRight, y, t->color, buffer);
                break;
            }

            gfx_drawLine(xLeft, y, xRight, y, t->color, buffer);
            xLeft  += dxLeft;
            xRight += dxRight;
        }
    }
    else
    {
        if(tm == AFFINE)
            affineTextureMap(t, v0, v1, v2, buffer, type, dxLeft, dxRight, yDir, invDy, colorKey);
        else
            perspectiveTextureMap(t, v0, v1, v2, buffer, type, dxLeft, dxRight, yDir, colorKey);
    }
}

static void perspectiveTextureMap(const gfx_Triangle *t, const gfx_Vertex *v0, const gfx_Vertex *v1, const gfx_Vertex *v2, unsigned char *buffer, enum TriangleType type, double dxLeft, double dxRight, double yDir, short colorKey)
{
    double x, y;
    int useColorKey = colorKey != -1 ? 1 : 0;
    float texW = t->texture->width - 1;
    float texH = t->texture->height - 1;
    int   texArea = texW * texH;

    float startX = v0->position.x;
    float endX   = startX;
    float invZ0, invZ1, invZ2;

    invZ0 = 1.f / v0->position.z;
    invZ1 = 1.f / v1->position.z;
    invZ2 = 1.f / v2->position.z;

    gfx_setPalette(t->texture->palette);

    for(y = v0->position.y; ; y += yDir)
    {
        float startInvZ, endInvZ, startU = texW, startV = texH, UEnd = texW, VEnd = texH, r1;

        if(type == FLAT_BOTTOM && y > v2->position.y)
        {
            /*for(x = startX-dxLeft; x <= endX-dxRight; ++x)
            {
                unsigned char pixel =  t->texture->data[((int)u + ((int)v * t->texture->height)) % texArea];

                if(!useColorKey || (useColorKey && pixel != (unsigned char)colorKey))
                    gfx_drawPixel(x, y, pixel, buffer);
                u += du;
                v += dv;
            }*/
            break;
        }
        else if ( type == FLAT_TOP && y < v2->position.y)
            break;

        r1 = (v0->position.y - y) / (v0->position.y - v2->position.y);
        startInvZ = LERP(invZ0, invZ2, r1);
        endInvZ = LERP(invZ0, invZ1, r1);

        startU *= LERP(v0->uv.u*invZ0, v2->uv.u*invZ2, r1);
        startV *= LERP(v0->uv.v*invZ0, v2->uv.v*invZ2, r1);
        UEnd *= LERP(v0->uv.u*invZ0, v1->uv.u*invZ1, r1);
        VEnd *= LERP(v0->uv.v*invZ0, v1->uv.v*invZ1, r1);

        for(x = startX; x <= endX; ++x)
        {
            float r = (x - startX) / (endX - startX);
            float lerpInvZ = LERP(startInvZ, endInvZ, r);
            float z = 1.f/lerpInvZ;
            float u = z * LERP(startU, UEnd, r);
            float v = z * LERP(startV, VEnd, r);

            // fetch texture data with a texArea modulus for proper effect in case u or v are > 1
            unsigned char pixel = t->texture->data[((int)u + ((int)v * t->texture->height)) % texArea];

            if(!useColorKey || (useColorKey && pixel != (unsigned char)colorKey))
                gfx_drawPixel(x, y, pixel, buffer);
        }

        startX += dxLeft;
        endX   += dxRight;
    }
}

static void affineTextureMap(const gfx_Triangle *t, const gfx_Vertex *v0, const gfx_Vertex *v1, const gfx_Vertex *v2, unsigned char *buffer, enum TriangleType type, double dxLeft, double dxRight, double yDir, double invDy, short colorKey)
{
    double x, y;
    int useColorKey = colorKey != -1 ? 1 : 0;
    float texW = t->texture->width - 1;
    float texH = t->texture->height - 1;
    int   texArea = texW * texH;
    float duLeft  = texW * (v2->uv.u - v0->uv.u) * invDy;
    float dvLeft  = texH * (v2->uv.v - v0->uv.v) * invDy;
    float duRight = texW * (v1->uv.u - v0->uv.u) * invDy;
    float dvRight = texH * (v1->uv.v - v0->uv.v) * invDy;

    float startU = texW * v0->uv.u;
    float startV = texH * v0->uv.v;
    // With triangles the texture gradients (u,v slopes over the triangle surface)
    // are guaranteed to be constant, so we need to calculate du and dv only once.
    float invDx = 1.f / (dxRight - dxLeft);
    float du = (duRight - duLeft) * invDx;
    float dv = (dvRight - dvLeft) * invDx;
    float startX = v0->position.x;
    float endX   = startX;

    gfx_setPalette(t->texture->palette);

    for(y = v0->position.y; ; y += yDir)
    {
        float u = startU;
        float v = startV;

        if(type == FLAT_BOTTOM && y > v2->position.y)
        {
            u -= du;
            v -= dv;
            /*for(x = startX-dxLeft; x <= endX-dxRight; ++x)
            {
                unsigned char pixel = t->texture->data[(int)u + ((int)v * t->texture->height)];

                if(!useColorKey || (useColorKey && pixel != (unsigned char)colorKey))
                    gfx_drawPixel(x, y, pixel, buffer);
                u += du;
                v += dv;
            }*/
            break;
        }
        else if ( type == FLAT_TOP && y < v2->position.y)
            break;

        for(x = startX; x <= endX; ++x)
        {
            // fetch texture data with a texArea modulus for proper effect in case u or v are > 1
            unsigned char pixel = t->texture->data[((int)u + ((int)v * t->texture->height)) % texArea];

            if(!useColorKey || (useColorKey && pixel != (unsigned char)colorKey))
                gfx_drawPixel(x, y, pixel, buffer);
            u += du;
            v += dv;
        }

        startX += dxLeft;
        endX   += dxRight;
        startU += duLeft;
        startV += dvLeft;
    }
}

