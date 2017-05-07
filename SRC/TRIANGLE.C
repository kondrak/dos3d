#include "src/triangle.h"

#define VERTEX_SWAP(v1, v2) { gfx_Vertex s = v2; v2 = v1; v1 = s; }

// simplest case: will plot either a flat bottom or flat top triangles
enum TriangleType
{
    FLAT_BOTTOM,
    FLAT_TOP
};

// internal: perform triangle rendering based on its type
static void __drawTriangleType(const gfx_Triangle *t, const gfx_Vertex *v0, const gfx_Vertex *v1, const gfx_Vertex *v2, unsigned char *buffer, enum TriangleType type, short colorKey);

/* ***** */
void gfx_drawTriangle(const gfx_Triangle *t, unsigned char *buffer)
{
    // draw triangle to screen buffer without any color keying
    gfx_drawTriangleColorKey(t, buffer, -1);
}

/* ***** */
void gfx_drawTriangleColorKey(const gfx_Triangle *t, unsigned char *buffer, short colorKey)
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
        __drawTriangleType(t, &v0, &v1, &v2, buffer, FLAT_BOTTOM, colorKey);
    else if(v0.position.y == v1.position.y)
        __drawTriangleType(t, &v2, &v1, &v0, buffer, FLAT_TOP, colorKey);
    else
    {
        // "Non-trivial" triangles will be broken down into a composition of flat bottom and flat top triangles.
        // For this, we need to calculate a new vertex, v3 and interpolate its UV coordinates.
        gfx_Vertex v3;
        mth_Vector4 diff, diff2;
        double ratioU = 1, ratioV = 1;

        v3.position.x = v0.position.x + ((float)(v2.position.y - v0.position.y) / (float)(v1.position.y - v0.position.y)) * (v1.position.x - v0.position.x);
        v3.position.y = v2.position.y;
        // todo: interpolate z for proper perspective texture mapping!
        v3.position.z = v2.position.z;
        v3.position.w = v2.position.w;

        diff  = mth_vecSub(&v1.position, &v0.position);
        diff2 = mth_vecSub(&v3.position, &v0.position);

        if(diff.x != 0)
            ratioU = diff2.x / diff.x;

        if(diff.y != 0)
            ratioV = diff2.y / diff.y;

        // lerp the UV mapping for the triangle
        v3.uv.u = v1.uv.u * ratioU + v0.uv.u * (1.0 - ratioU);
        v3.uv.v = v1.uv.v * ratioV + v0.uv.v * (1.0 - ratioV);

        // this swap is done to maintain consistent renderer behavior
        if(v3.position.x < v2.position.x)
            VERTEX_SWAP(v3, v2)

        // draw the composition of both triangles to form the desired shape
        __drawTriangleType(t, &v0, &v3, &v2, buffer, FLAT_BOTTOM, colorKey);
        __drawTriangleType(t, &v1, &v3, &v2, buffer, FLAT_TOP, colorKey);
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
static void __drawTriangleType(const gfx_Triangle *t, const gfx_Vertex *v0, const gfx_Vertex *v1, const gfx_Vertex *v2, unsigned char *buffer, enum TriangleType type, const short colorKey)
{
    double invDy, dxLeft, dxRight, xLeft, xRight;
    double x, y, yDir = 1;
    int useColorKey = colorKey != -1 ? 1 : 0;

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
        float texW = t->texture->width - 1;
        float texH = t->texture->height - 1;
        int   texArea = texW * texH;
        float duLeft  = texW * (v2->uv.u - v0->uv.u) * invDy;
        float dvLeft  = texH * (v2->uv.v - v0->uv.v) * invDy;
        float duRight = texW * (v1->uv.u - v0->uv.u) * invDy;
        float dvRight = texH * (v1->uv.v - v0->uv.v) * invDy;

        float uLeft = texW * v0->uv.u;
        float uRight = uLeft;
        float vLeft = texH * v0->uv.v;
        float vRight = vLeft;

        gfx_setPalette(t->texture->palette);

        for(y = v0->position.y; ; y += yDir)
        {
            int startX = xLeft;
            int endX = xRight;
            float u = uLeft;
            float v = vLeft;
            float du, dv;
            float dx = endX - startX;

            if(type == FLAT_BOTTOM && y > v2->position.y)
            {
                u -= du;
                v -= dv;
                for(x = startX-dxLeft; x <= endX-dxRight; ++x)
                {
                    unsigned char pixel = t->texture->data[(int)u + ((int)v * t->texture->height)];

                    if(!useColorKey || (useColorKey && pixel != (unsigned char)colorKey))
                        gfx_drawPixel(x, y, pixel, buffer);
                    u += du;
                    v += dv;
                }
                break;
            }
            else if ( type == FLAT_TOP && y < v2->position.y)
                break;

            if(dx > 0)
            {
                du = (uRight - uLeft) / dx;
                dv = (vRight - vLeft) / dx;
            }
            else
            {
                du = uRight - uLeft;
                dv = vRight - vLeft;
            }

            for(x = startX; x <= endX; ++x)
            {
                // fetch texture data with a texArea modulus for proper effect in case u or v are > 1
                unsigned char pixel = t->texture->data[((int)u + ((int)v * t->texture->height)) % texArea];

                if(!useColorKey || (useColorKey && pixel != (unsigned char)colorKey))
                    gfx_drawPixel(x, y, pixel, buffer);
                u += du;
                v += dv;
            }

            xLeft  += dxLeft;
            xRight += dxRight;
            uLeft  += duLeft;
            uRight += duRight;
            vLeft  += dvLeft;
            vRight += dvRight;
        }
    }
}
