#include "src/fillers.h"
#include "src/utils.h"
#include <memory.h>

/* ***** */
void gfx_wireFrame(const gfx_Triangle *t, gfx_drawBuffer *target)
{
    gfx_drawLine(t->vertices[0].position.x, t->vertices[0].position.y, t->vertices[0].position.z, 
                 t->vertices[1].position.x, t->vertices[1].position.y, t->vertices[1].position.z, t->color, target);
    gfx_drawLine(t->vertices[1].position.x, t->vertices[1].position.y, t->vertices[1].position.z, 
                 t->vertices[2].position.x, t->vertices[2].position.y, t->vertices[2].position.z, t->color, target);
    gfx_drawLine(t->vertices[2].position.x, t->vertices[2].position.y, t->vertices[2].position.z, 
                 t->vertices[0].position.x, t->vertices[0].position.y, t->vertices[0].position.z, t->color, target);
}

/* ***** */
void gfx_flatFill(const gfx_Triangle *t, gfx_drawBuffer *target, enum TriangleType type)
{
    const gfx_Vertex *v0 = &t->vertices[0];
    const gfx_Vertex *v1 = &t->vertices[1];
    const gfx_Vertex *v2 = &t->vertices[2];
    double y, invDy, dxLeft, dxRight, xLeft, xRight, prestep;
    int currLine, numScanlines, x0, x1, yDir = 1;
    // variables used if depth test is enabled
    float startInvZ, endInvZ, invZ0, invZ1, invZ2, invY02;

    if(type == FLAT_BOTTOM)
    {
        invDy  = 1.0 / (v2->position.y - v0->position.y);
        numScanlines = ceil(v2->position.y) - ceil(v0->position.y);
        prestep = ceil(v0->position.y) - v0->position.y;
        // todo: handle line sizes of height < 1
        if(v2->position.y - v0->position.y < 1) return;
    }
    else
    {
        invDy  = 1.0 / (v0->position.y - v2->position.y);
        yDir = -1;
        numScanlines = ceil(v0->position.y) - ceil(v2->position.y);
        prestep = ceil(v2->position.y) - v2->position.y;
        // todo: handle line sizes of height < 1
        if(v0->position.y - v2->position.y < 1) return;
    }

    dxLeft  = (v2->position.x - v0->position.x) * invDy;
    dxRight = (v1->position.x - v0->position.x) * invDy;
    xLeft   = v0->position.x + dxLeft * prestep;
    xRight  = v0->position.x + dxRight * prestep;

    // skip unnecessary divisions if there's no depth testing
    if(target->drawOpts.depthFunc != DF_ALWAYS)
    {
        invZ0  = 1.f / v0->position.z;
        invZ1  = 1.f / v1->position.z;
        invZ2  = 1.f / v2->position.z;
        invY02 = 1.f / (v0->position.y - v2->position.y);
    }

    for(currLine = 0, y = ceil(v0->position.y); currLine <= numScanlines; y += yDir)
    {
        x0 = ceil(xLeft);
        x1 = ceil(xRight);

        // interpolate 1/z only if depth testing is enabled
        if(target->drawOpts.depthFunc != DF_ALWAYS)
        {
            float r1  = (v0->position.y - y) * invY02;
            startInvZ = LERP(invZ0, invZ2, r1);
            endInvZ   = LERP(invZ0, invZ1, r1);
            gfx_drawLine(x0, y, 1.f/startInvZ, x1, y, 1.f/endInvZ, t->color, target);
        }
        else
        {
            if(x0 > x1) SWAP(x0, x1);
            memset(target->colorBuffer + x0 + (int)y * target->width, t->color, sizeof(uint8_t) * (x1 - x0 + 1));
        }

        if(++currLine < numScanlines)
        {
            xLeft  += dxLeft;
            xRight += dxRight;
        }
    }
}

/* ***** */
void gfx_perspectiveTextureMap(const gfx_Triangle *t, gfx_drawBuffer *target, enum TriangleType type)
{
    const gfx_Vertex *v0 = &t->vertices[0];
    const gfx_Vertex *v1 = &t->vertices[1];
    const gfx_Vertex *v2 = &t->vertices[2];
    double x, y, invDy, dxLeft, dxRight, prestep, yDir = 1;
    double startX, endX, startXPrestep, endXPrestep, lineLength;
    int   useColorKey = target->drawOpts.colorKey >= 0 ? 1 : 0;
    int   texW = t->texture->width - 1;
    int   texH = t->texture->height - 1;
    int   texArea = texW * texH;
    int   currLine, numScanlines;
    float invZ0, invZ1, invZ2, invY02 = 1.f;

    if(type == FLAT_BOTTOM)
    {
        invDy  = 1.0 / (v2->position.y - v0->position.y);
        numScanlines = ceil(v2->position.y) - ceil(v0->position.y);
        prestep = ceil(v0->position.y) - v0->position.y;
        // todo: handle line sizes of height < 1
        if(v2->position.y - v0->position.y < 1) return;
    }
    else
    {
        invDy  = 1.0 / (v0->position.y - v2->position.y);
        yDir = -1;
        numScanlines = ceil(v0->position.y) - ceil(v2->position.y);
        prestep = ceil(v2->position.y) - v2->position.y;
        // todo: handle line sizes of height < 1
        if(v0->position.y - v2->position.y < 1) return;
    }

    dxLeft  = (v2->position.x - v0->position.x) * invDy;
    dxRight = (v1->position.x - v0->position.x) * invDy;
    startX  = v0->position.x;
    endX    = startX;
    startXPrestep = v0->position.x + dxLeft * prestep;
    endXPrestep   = v0->position.x + dxRight * prestep;

    invZ0  = 1.f / v0->position.z;
    invZ1  = 1.f / v1->position.z;
    invZ2  = 1.f / v2->position.z;
    invY02 = 1.f / (v0->position.y - v2->position.y);

    for(currLine = 0, y = v0->position.y; currLine <= numScanlines; y += yDir)
    {
        float startInvZ, endInvZ, r1, invLineLength;
        float startU = texW, startV = texH, endU = texW, endV = texH;
        lineLength = endX - startX;

        r1 = (v0->position.y - y) * invY02;
        startInvZ = LERP(invZ0, invZ2, r1);
        endInvZ   = LERP(invZ0, invZ1, r1);

        startU *= LERP(v0->uv.u * invZ0, v2->uv.u * invZ2, r1);
        startV *= LERP(v0->uv.v * invZ0, v2->uv.v * invZ2, r1);
        endU   *= LERP(v0->uv.u * invZ0, v1->uv.u * invZ1, r1);
        endV   *= LERP(v0->uv.v * invZ0, v1->uv.v * invZ1, r1);

        // skip zero-length lines
        if(lineLength > 0)
        {
            invLineLength = 1.f / lineLength;

            for(x = startXPrestep; x <= endXPrestep; ++x)
            {
                // interpolate 1/z for each pixel in the scanline
                float r = (x - startX) * invLineLength;
                float lerpInvZ = LERP(startInvZ, endInvZ, r);
                float z = 1.f/lerpInvZ;
                float u = MAX(0, z * LERP(startU, endU, r));
                float v = MAX(0, z * LERP(startV, endV, r));

                // fetch texture data with a texArea modulus for proper effect in case u or v are > 1
                uint8_t pixel = t->texture->data[((uint16_t)u + (uint16_t)v * t->texture->height) % texArea];

                if(!useColorKey || (useColorKey && pixel != (uint8_t)target->drawOpts.colorKey))
                {
                    // DF_ALWAYS = no depth test
                    if(target->drawOpts.depthFunc == DF_ALWAYS)
                        gfx_drawPixel(ceil(x), ceil(y), pixel, target);
                    else
                        gfx_drawPixelWithDepth(ceil(x), ceil(y), lerpInvZ, pixel, target);
                }
            }
        }

        startX += dxLeft;
        endX   += dxRight;

        if(++currLine < numScanlines)
        {
            startXPrestep += dxLeft;
            endXPrestep   += dxRight;
        }
    }
}

/* ***** */
void gfx_affineTextureMap(const gfx_Triangle *t, gfx_drawBuffer *target, enum TriangleType type)
{
    const gfx_Vertex *v0 = &t->vertices[0];
    const gfx_Vertex *v1 = &t->vertices[1];
    const gfx_Vertex *v2 = &t->vertices[2];
    double x, y, invDy, dxLeft, dxRight, prestep, yDir = 1;
    double startU, startV, invDx, du, dv, lineLength;
    double startX, endX, startXPrestep, endXPrestep;
    float duLeft, dvLeft, duRight, dvRight;
    float texW = t->texture->width - 1;
    float texH = t->texture->height - 1;
    int   useColorKey = target->drawOpts.colorKey >= 0 ? 1 : 0;
    int   texArea = texW * texH;
    int   currLine, numScanlines;
    // variables used only if depth test is enabled
    float invZ0, invZ1, invZ2, invY02 = 1.f;

    if(type == FLAT_BOTTOM)
    {
        invDy  = 1.f / (v2->position.y - v0->position.y);
        numScanlines = ceil(v2->position.y) - ceil(v0->position.y);
        prestep = ceil(v0->position.y) - v0->position.y;
        // todo: handle line sizes of height < 1
        if(v2->position.y - v0->position.y < 1) return;
    }
    else
    {
        invDy  = 1.f / (v0->position.y - v2->position.y);
        yDir = -1;
        numScanlines = ceil(v0->position.y) - ceil(v2->position.y);
        prestep = ceil(v2->position.y) - v2->position.y;
        // todo: handle line sizes of height < 1
        if(v0->position.y - v2->position.y < 1) return;
    }

    dxLeft  = (v2->position.x - v0->position.x) * invDy;
    dxRight = (v1->position.x - v0->position.x) * invDy;

    duLeft  = texW * (v2->uv.u - v0->uv.u) * invDy;
    dvLeft  = texH * (v2->uv.v - v0->uv.v) * invDy;
    duRight = texW * (v1->uv.u - v0->uv.u) * invDy;
    dvRight = texH * (v1->uv.v - v0->uv.v) * invDy;

    startU = texW * v0->uv.u + duLeft * prestep;
    startV = texH * v0->uv.v + dvLeft * prestep;
    // With triangles, the texture gradients (u,v slopes over the triangle surface)
    // are guaranteed to be constant, so we need to calculate du and dv only once.
    invDx = 1.f / (dxRight - dxLeft);
    du = (duRight - duLeft) * invDx;
    dv = (dvRight - dvLeft) * invDx;
    startX = v0->position.x;
    endX   = startX;
    startXPrestep = v0->position.x + dxLeft * prestep;
    endXPrestep   = v0->position.x + dxRight * prestep;

    // skip unnecessary divisions if there's no depth testing
    if(target->drawOpts.depthFunc != DF_ALWAYS)
    {
        invZ0  = 1.f / v0->position.z;
        invZ1  = 1.f / v1->position.z;
        invZ2  = 1.f / v2->position.z;
        invY02 = 1.f / (v0->position.y - v2->position.y);
    }

    for(currLine = 0, y = v0->position.y; currLine <= numScanlines; y += yDir)
    {
        float u = startU;
        float v = startV;
        // variables used only if depth test is enabled
        float startInvZ, endInvZ, invLineLength;
        lineLength = endX - startX;

        // interpolate 1/z only if depth testing is enabled
        if(target->drawOpts.depthFunc != DF_ALWAYS)
        {
            float r1  = (v0->position.y - y) * invY02;
            startInvZ = LERP(invZ0, invZ2, r1);
            endInvZ   = LERP(invZ0, invZ1, r1);

            if(lineLength)
                invLineLength = 1.f / lineLength;
        }

        // skip zero-length lines
        if(lineLength > 0)
        {
            for(x = startXPrestep; x <= endXPrestep; ++x)
            {
                // fetch texture data with a texArea modulus for proper effect in case u or v are > 1
                uint8_t pixel = t->texture->data[((uint16_t)u + (uint16_t)v * t->texture->height) % texArea];

                if(!useColorKey || (useColorKey && pixel != (uint8_t)target->drawOpts.colorKey))
                {
                    // DF_ALWAYS = no depth test
                    if(target->drawOpts.depthFunc == DF_ALWAYS)
                        gfx_drawPixel(ceil(x), ceil(y), pixel, target);
                    else
                    {
                        float r = (x - startX) * invLineLength;
                        float lerpInvZ = LERP(startInvZ, endInvZ, r);
                        gfx_drawPixelWithDepth(ceil(x), ceil(y), lerpInvZ, pixel, target);
                    }
                }
                u += du;
                v += dv;
            }
        }

        startX += dxLeft;
        endX   += dxRight;

        if(++currLine < numScanlines)
        {
            startXPrestep += dxLeft;
            endXPrestep   += dxRight;
            startU += duLeft;
            startV += dvLeft;
        }
    }
}
