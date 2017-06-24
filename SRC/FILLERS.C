#include "src/fillers.h"
#include "src/utils.h"

/* ***** */
void gfx_flatFill(const gfx_Triangle *t, gfx_drawBuffer *buffer, enum TriangleType type)
{
    const gfx_Vertex *v0 = &t->vertices[0];
    const gfx_Vertex *v1 = &t->vertices[1];
    const gfx_Vertex *v2 = &t->vertices[2];
    double y, invDy, dxLeft, dxRight, xLeft, xRight;
    int currLine, numScanlines, yDir = 1;
    // variables used if depth test is enabled
    float startInvZ, endInvZ, invZ0, invZ1, invZ2, invY02;

    if(type == FLAT_BOTTOM)
    {
        invDy  = 1.0 / (v2->position.y - v0->position.y);
        numScanlines = ceil(v2->position.y) - ceil(v0->position.y);
    }
    else
    {
        invDy  = 1.0 / (v0->position.y - v2->position.y);
        yDir = -1;
        numScanlines = ceil(v0->position.y) - ceil(v2->position.y);
    }

    dxLeft  = (v2->position.x - v0->position.x) * invDy;
    dxRight = (v1->position.x - v0->position.x) * invDy;
    xLeft   = v0->position.x;
    xRight  = xLeft;

    // skip the unnecessary divisions if there's no depth testing
    if(buffer->drawOpts.depthFunc != DF_ALWAYS)
    {
        invZ0  = 1.f / v0->position.z;
        invZ1  = 1.f / v1->position.z;
        invZ2  = 1.f / v2->position.z;
        invY02 = 1.f / (v0->position.y - v2->position.y);
    }

    for(currLine = 0, y = v0->position.y; currLine <= numScanlines; y += yDir)
    {
        // interpolate 1/z only if depth testing is enabled
        if(buffer->drawOpts.depthFunc != DF_ALWAYS)
        {
            float r1  = (v0->position.y - y) * invY02;
            startInvZ = LERP(invZ0, invZ2, r1);
            endInvZ   = LERP(invZ0, invZ1, r1);
            gfx_drawLine(xLeft, y, 1.f/startInvZ, xRight, y, 1.f/endInvZ, t->color, buffer);
        }
        else
            gfx_drawLine(xLeft, y, 0.f, xRight, y, 0.f, t->color, buffer);

        xLeft  += dxLeft;
        xRight += dxRight;

        if(++currLine == numScanlines)
        {
            xLeft -= dxLeft;
            xRight -= dxRight;
        }
    }
}

/* ***** */
void gfx_perspectiveTextureMap(const gfx_Triangle *t, gfx_drawBuffer *buffer, enum TriangleType type)
{
    const gfx_Vertex *v0 = &t->vertices[0];
    const gfx_Vertex *v1 = &t->vertices[1];
    const gfx_Vertex *v2 = &t->vertices[2];
    double x, y, invDy, dxLeft, dxRight, yDir = 1;
    int   useColorKey = buffer->drawOpts.colorKey != -1 ? 1 : 0;
    int   texW = t->texture->width - 1;
    int   texH = t->texture->height - 1;
    int   texArea = texW * texH;
    float startX  = v0->position.x;
    float endX    = startX;
    float invZ0, invZ1, invZ2, invY02 = 1.f;
    int   currLine, numScanlines;

    if(type == FLAT_BOTTOM)
    {
        invDy  = 1.0 / (v2->position.y - v0->position.y);
        numScanlines = ceil(v2->position.y) - ceil(v0->position.y);
    }
    else
    {
        invDy  = 1.0 / (v0->position.y - v2->position.y);
        numScanlines = ceil(v0->position.y) - ceil(v2->position.y);
        yDir = -1;
    }

    dxLeft  = (v2->position.x - v0->position.x) * invDy;
    dxRight = (v1->position.x - v0->position.x) * invDy;

    invZ0  = 1.f / v0->position.z;
    invZ1  = 1.f / v1->position.z;
    invZ2  = 1.f / v2->position.z;
    invY02 = 1.f / (v0->position.y - v2->position.y);

    for(currLine = 0, y = v0->position.y; currLine <= numScanlines; y += yDir)
    {
        float startInvZ, endInvZ, r1, invLineLength = 0.f;
        float startU = texW, startV = texH, endU = texW, endV = texH;

        r1 = (v0->position.y - y) * invY02;
        startInvZ = LERP(invZ0, invZ2, r1);
        endInvZ   = LERP(invZ0, invZ1, r1);

        startU *= LERP(v0->uv.u * invZ0, v2->uv.u * invZ2, r1);
        startV *= LERP(v0->uv.v * invZ0, v2->uv.v * invZ2, r1);
        endU   *= LERP(v0->uv.u * invZ0, v1->uv.u * invZ1, r1);
        endV   *= LERP(v0->uv.v * invZ0, v1->uv.v * invZ1, r1);

        if(startX != endX)
            invLineLength = 1.f / (endX - startX);

        for(x = startX; x <= endX; ++x)
        {
            // interpolate 1/z for each pixel in the scanline
            float r = (x - startX) * invLineLength;
            float lerpInvZ = LERP(startInvZ, endInvZ, r);
            float z = 1.f/lerpInvZ;
            float u = z * LERP(startU, endU, r);
            float v = z * LERP(startV, endV, r);

            // fetch texture data with a texArea modulus for proper effect in case u or v are > 1
            unsigned char pixel = t->texture->data[((int)u + ((int)v * t->texture->height)) % texArea];

            if(!useColorKey || (useColorKey && pixel != (unsigned char)buffer->drawOpts.colorKey))
            {
                // DF_ALWAYS = no depth test
                if(buffer->drawOpts.depthFunc == DF_ALWAYS)
                    gfx_drawPixel(x, y, pixel, buffer);
                else
                    gfx_drawPixelWithDepth(x, y, lerpInvZ, pixel, buffer);
            }
        }

        startX += dxLeft;
        endX   += dxRight;

        if(++currLine == numScanlines)
        {
            startX -= dxLeft;
            endX   -= dxRight;
        }
    }
}

/* ***** */
void gfx_affineTextureMap(const gfx_Triangle *t, gfx_drawBuffer *buffer, enum TriangleType type)
{
    const gfx_Vertex *v0 = &t->vertices[0];
    const gfx_Vertex *v1 = &t->vertices[1];
    const gfx_Vertex *v2 = &t->vertices[2];
    double x, y, invDy, dxLeft, dxRight, yDir = 1;
    int   useColorKey = buffer->drawOpts.colorKey != -1 ? 1 : 0;
    float duLeft, dvLeft, duRight, dvRight;
    float startU, startV, invDx, du, dv, startX, endX;
    float texW = t->texture->width - 1;
    float texH = t->texture->height - 1;
    int   texArea = texW * texH;
    // variables used only if depth test is enabled
    float invZ0, invZ1, invZ2, invY02 = 1.f;
    int   currLine, numScanlines;

    if(type == FLAT_BOTTOM)
    {
        invDy  = 1.f / (v2->position.y - v0->position.y);
        numScanlines = ceil(v2->position.y) - ceil(v0->position.y);
    }
    else
    {
        invDy  = 1.f / (v0->position.y - v2->position.y);
        numScanlines = ceil(v0->position.y) - ceil(v2->position.y);
        yDir = -1;
    }

    dxLeft  = (v2->position.x - v0->position.x) * invDy;
    dxRight = (v1->position.x - v0->position.x) * invDy;

    duLeft  = texW * (v2->uv.u - v0->uv.u) * invDy;
    dvLeft  = texH * (v2->uv.v - v0->uv.v) * invDy;
    duRight = texW * (v1->uv.u - v0->uv.u) * invDy;
    dvRight = texH * (v1->uv.v - v0->uv.v) * invDy;

    startU = texW * v0->uv.u;
    startV = texH * v0->uv.v;
    // With triangles the texture gradients (u,v slopes over the triangle surface)
    // are guaranteed to be constant, so we need to calculate du and dv only once.
    invDx = 1.f / (dxRight - dxLeft);
    du = (duRight - duLeft) * invDx;
    dv = (dvRight - dvLeft) * invDx;
    startX = v0->position.x;
    endX   = startX;

    // skip the unnecessary divisions if there's no depth testing
    if(buffer->drawOpts.depthFunc != DF_ALWAYS)
    {
        invZ0  = 1.f / v0->position.z;
        invZ1  = 1.f / v1->position.z;
        invZ2  = 1.f / v2->position.z;
        invY02 = 1.f / (v0->position.y - v2->position.y);
    }

    for(currLine = 0, y = v0->position.y; currLine <= numScanlines ; y += yDir)
    {
        float u = startU;
        float v = startV;
        // variables used only if depth test is enabled
        float startInvZ, endInvZ, invLineLength = 0.f;

        // interpolate 1/z only if depth testing is enabled
        if(buffer->drawOpts.depthFunc != DF_ALWAYS)
        {
            float r1  = (v0->position.y - y) * invY02;
            startInvZ = LERP(invZ0, invZ2, r1);
            endInvZ   = LERP(invZ0, invZ1, r1);

            if(startX != endX)
                invLineLength = 1.f / (endX - startX);
        }

        for(x = startX; x <= endX; ++x)
        {
            // fetch texture data with a texArea modulus for proper effect in case u or v are > 1
            unsigned char pixel = t->texture->data[((int)u + ((int)v * t->texture->height)) % texArea];

            if(!useColorKey || (useColorKey && pixel != (unsigned char)buffer->drawOpts.colorKey))
            {
                // DF_ALWAYS = no depth test
                if(buffer->drawOpts.depthFunc == DF_ALWAYS)
                    gfx_drawPixel(x, y, pixel, buffer);
                else
                {
                    float r = (x - startX) * invLineLength;
                    float lerpInvZ = LERP(startInvZ, endInvZ, r);
                    gfx_drawPixelWithDepth(x, y, lerpInvZ, pixel, buffer);
                }
            }
            u += du;
            v += dv;
        }

        startX += dxLeft;
        endX   += dxRight;
        startU += duLeft;
        startV += dvLeft;

        if(++currLine == numScanlines)
        {
            startX -= dxLeft;
            endX   -= dxRight;
            startU -= duLeft;
            startV -= dvLeft;
        }
    }
}
