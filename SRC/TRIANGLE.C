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
static void drawTriangleType(const gfx_Triangle *t, const gfx_Vertex *v0, const gfx_Vertex *v1, const gfx_Vertex *v2, gfx_drawBuffer *buffer, enum TriangleType type);
static void perspectiveTextureMap(const gfx_Bitmap *tex, const gfx_Vertex *v0, const gfx_Vertex *v1, const gfx_Vertex *v2, gfx_drawBuffer *buffer, enum TriangleType type);
static void affineTextureMap(const gfx_Bitmap *tex, const gfx_Vertex *v0, const gfx_Vertex *v1, const gfx_Vertex *v2, gfx_drawBuffer *buffer, enum TriangleType type);

#define DEGENERATE(v0, v1, v2) ( (v0.position.x == v1.position.x && v0.position.x == v2.position.x) || \
                                 (v0.position.y == v1.position.y && v0.position.y == v2.position.y) )

#define COORD_CLIPPED(p0, p1, p2, c) ( (p0.c < -p0.w && p1.c < -p1.w && p2.c < -p2.w) || \
                                       (p0.c >  p0.w && p1.c >  p1.w && p2.c >  p2.w ) )
#define X_CLIPPED(p0, p1, p2)        COORD_CLIPPED(p0, p1, p2, x)
#define Y_CLIPPED(p0, p1, p2)        COORD_CLIPPED(p0, p1, p2, y)
#define Z_CLIPPED(p0, p1, p2)        ( (p0.z < 0    && p1.z < 0    && p2.z < 0) || \
                                       (p0.z > p0.w && p1.z > p1.w && p2.z > p2.w) )

#define TRIANGLE_OFFSCREEN(v0, v1, v2) ( X_CLIPPED(v0.position, v1.position, v2.position) || \
                                         Y_CLIPPED(v0.position, v1.position, v2.position) || \
                                         Z_CLIPPED(v0.position, v1.position, v2.position) )
/* ***** */
void gfx_drawTriangle(const gfx_Triangle *t, const mth_Matrix4 *matrix, gfx_drawBuffer *buffer)
{
    int bufferHalfWidth  = buffer->width >> 1;
    int bufferHalfHeight = buffer->height >> 1;
    gfx_Vertex v0, v1, v2;

    v0 = t->vertices[0];
    v1 = t->vertices[1];
    v2 = t->vertices[2];

    // DF_NEVER - don't draw anything
    if(buffer->drawOpts.depthFunc == DF_NEVER)
        return;

    // transform the vertices
    v0.position = mth_matMulVec(matrix, &v0.position);
    v1.position = mth_matMulVec(matrix, &v1.position);
    v2.position = mth_matMulVec(matrix, &v2.position);

    // skip rendering if triangle is completely offscreen
    if(TRIANGLE_OFFSCREEN(v0, v1, v2))
        return;

    // test if triangle face should be back/front face culled
    if(buffer->drawOpts.cullMode != FC_NONE)
    {
        mth_Vector4 d1 = mth_vecSub(&v1.position, &v0.position);
        mth_Vector4 d2 = mth_vecSub(&v2.position, &v0.position);
        mth_Vector4 n  = mth_crossProduct(&d1, &d2);
        double dp = mth_dotProduct(&v0.position, &n);
        
        if(buffer->drawOpts.cullMode == FC_BACK && dp >= 0)
            return;
        else if(buffer->drawOpts.cullMode == FC_FRONT && dp < 0)
            return;
    }

    // transform x and y of each vertex to screen coordinates
    v0.position.x = (v0.position.x * (float)buffer->width)  / (2.0f * v0.position.w) + bufferHalfWidth;
    v0.position.y = (v0.position.y * (float)buffer->height) / (2.0f * v0.position.w) + bufferHalfHeight;
    v1.position.x = (v1.position.x * (float)buffer->width)  / (2.0f * v1.position.w) + bufferHalfWidth;
    v1.position.y = (v1.position.y * (float)buffer->height) / (2.0f * v1.position.w) + bufferHalfHeight;
    v2.position.x = (v2.position.x * (float)buffer->width)  / (2.0f * v2.position.w) + bufferHalfWidth;
    v2.position.y = (v2.position.y * (float)buffer->height) / (2.0f * v2.position.w) + bufferHalfHeight;

    // sort vertices so that v0 is topmost, then v2, then v1
    if(v2.position.y > v1.position.y)
        VERTEX_SWAP(v1, v2)

    if(v0.position.y > v1.position.y)
        VERTEX_SWAP(v0, v1)

    if(v0.position.y > v2.position.y)
        VERTEX_SWAP(v0, v2)

    // discard degenerate triangles
    if(DEGENERATE(v0, v1, v2))
        return;

    // handle 2 basic cases of flat bottom and flat top triangles
    if(v1.position.y == v2.position.y)
        drawTriangleType(t, &v0, &v1, &v2, buffer, FLAT_BOTTOM);
    else if(v0.position.y == v1.position.y)
        drawTriangleType(t, &v2, &v1, &v0, buffer, FLAT_TOP);
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

        // lerp Z and UV for v3. For perspective texture mapping calculate u/z, v/z, for affine skip unnecessary divisions;
        // perform this step for affine texture mapping if depth testing is enabled, since then correct Z is needed for v3!
        if(buffer->drawOpts.texMapMode != TM_AFFINE || buffer->drawOpts.depthFunc != DF_ALWAYS)
        {
            float invV0Z = 1.f/v0.position.z;
            float invV1Z = 1.f/v1.position.z;

            // get the z value for v3 by interpolating 1/z, since that can be done using lerp (at this point we skip v3.w - it won't be relevant anymore)
            if((v0.position.x - v1.position.x) != 0.0)
                v3.position.z = 1.0 / LERP(invV1Z, invV0Z, (v3.position.x - v1.position.x) / (v0.position.x - v1.position.x));
            else
                v3.position.z = v0.position.z;

            // this will affect how affine texture map looks on the final polygon if depth test is enabled!
            v3.uv.u = v3.position.z * LERP(v0.uv.u*invV0Z, v1.uv.u*invV1Z, ratioU);
            v3.uv.v = v3.position.z * LERP(v0.uv.v*invV0Z, v1.uv.v*invV1Z, ratioV);
        }
        else
        {
            // simple Intercept Theorem is fine in case of affine texture mapping if depth testing is inactive (skip v3.w again)
            v3.position.z = v0.position.z + ((float)(v2.position.y - v0.position.y) / (float)(v1.position.y - v0.position.y)) * (v1.position.z - v0.position.z);
            v3.uv.u = LERP(v0.uv.u, v1.uv.u, ratioU);
            v3.uv.v = LERP(v0.uv.v, v1.uv.v, ratioV);
        }

        // this swap is done to maintain consistent renderer behavior
        if(v3.position.x < v2.position.x)
            VERTEX_SWAP(v3, v2)

        // draw the composition of both triangles to form the desired shape
        if(!DEGENERATE(v0, v3, v2))
            drawTriangleType(t, &v0, &v3, &v2, buffer, FLAT_BOTTOM);

        if(!DEGENERATE(v1, v3, v2))
            drawTriangleType(t, &v1, &v3, &v2, buffer, FLAT_TOP);
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
static void drawTriangleType(const gfx_Triangle *t, const gfx_Vertex *v0, const gfx_Vertex *v1, const gfx_Vertex *v2, gfx_drawBuffer *buffer, enum TriangleType type)
{
    if(!t->texture)
    {
        double invDy, dxLeft, dxRight, xLeft, xRight;
        // variables used if depth test is enabled
        float startInvZ, endInvZ, invZ0, invZ1, invZ2, invY02;
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

        // skip the unnecessary divisions if there's no depth testing
        if(buffer->drawOpts.depthFunc != DF_ALWAYS)
        {
            invZ0  = 1.f / v0->position.z;
            invZ1  = 1.f / v1->position.z;
            invZ2  = 1.f / v2->position.z;
            invY02 = 1.f / (v0->position.y - v2->position.y);
        }

        for(y = v0->position.y; ; y += yDir)
        {
            if(type == FLAT_TOP && y < v2->position.y)
            {
                break;
            }
            else if(type == FLAT_BOTTOM && y > v2->position.y)
            {
                // to avoid pixel wide gaps, render extra line at the junction between two final points
                if(buffer->drawOpts.depthFunc != DF_ALWAYS)
                    gfx_drawLine(xLeft-dxLeft, y, 1.f/startInvZ, xRight-dxRight, y, 1.f/endInvZ, t->color, buffer);
                else
                    gfx_drawLine(xLeft-dxLeft, y, 0.f, xRight-dxRight, y, 0.f, t->color, buffer);
                break;
            }

            // skip if no depth testing
            if(buffer->drawOpts.depthFunc != DF_ALWAYS)
            {
                float r1 = (v0->position.y - y) * invY02;
                startInvZ = LERP(invZ0, invZ2, r1);
                endInvZ   = LERP(invZ0, invZ1, r1);
                gfx_drawLine(xLeft, y, 1.f/startInvZ, xRight, y, 1.f/endInvZ, t->color, buffer);
            }
            else
                gfx_drawLine(xLeft, y, 0.f, xRight, y, 0.f, t->color, buffer);

            xLeft  += dxLeft;
            xRight += dxRight;
        }
    }
    else
    {
        if(buffer->drawOpts.texMapMode == TM_AFFINE)
            affineTextureMap(t->texture, v0, v1, v2, buffer, type);
        else
            perspectiveTextureMap(t->texture, v0, v1, v2, buffer, type);
    }
}

static void perspectiveTextureMap(const gfx_Bitmap *tex, const gfx_Vertex *v0, const gfx_Vertex *v1, const gfx_Vertex *v2, gfx_drawBuffer *buffer, enum TriangleType type)
{
    double x, y, invDy, dxLeft, dxRight, yDir = 1;

    int   useColorKey = buffer->drawOpts.colorKey != -1 ? 1 : 0;
    int   texW = tex->width - 1;
    int   texH = tex->height - 1;
    int   texArea = texW * texH;
    float startX = v0->position.x;
    float endX   = startX;
    float invZ0, invZ1, invZ2, invY02 = 1.f;
    int   finished = 0;

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

    invZ0  = 1.f / v0->position.z;
    invZ1  = 1.f / v1->position.z;
    invZ2  = 1.f / v2->position.z;
    invY02 = 1.f / (v0->position.y - v2->position.y);

    for(y = v0->position.y; ; y += yDir)
    {
        float startInvZ, endInvZ, r1, invLineLength = 0.f;
        float startU = texW, startV = texH, endU = texW, endV = texH;

        if(type == FLAT_BOTTOM && y > v2->position.y)
        {
            // in final iteration draw extra scanline to avoid pixel wide gaps
            startX -= dxLeft;
            endX   -= dxRight;
            y = v2->position.y;
            finished = 1;
        }
        else if ( type == FLAT_TOP && y < v2->position.y)
            break;

        r1 = (v0->position.y - y) * invY02;
        startInvZ = LERP(invZ0, invZ2, r1);
        endInvZ   = LERP(invZ0, invZ1, r1);

        startU *= LERP(v0->uv.u*invZ0, v2->uv.u*invZ2, r1);
        startV *= LERP(v0->uv.v*invZ0, v2->uv.v*invZ2, r1);
        endU *= LERP(v0->uv.u*invZ0, v1->uv.u*invZ1, r1);
        endV *= LERP(v0->uv.v*invZ0, v1->uv.v*invZ1, r1);

        if(startX != endX)
            invLineLength = 1.f / (endX - startX);

        for(x = startX; x <= endX; ++x)
        {
            float r = (x - startX) * invLineLength;
            float lerpInvZ = LERP(startInvZ, endInvZ, r);
            float z = 1.f/lerpInvZ;
            float u = z * LERP(startU, endU, r);
            float v = z * LERP(startV, endV, r);

            // fetch texture data with a texArea modulus for proper effect in case u or v are > 1
            unsigned char pixel = tex->data[((int)u + ((int)v * tex->height)) % texArea];

            if(!useColorKey || (useColorKey && pixel != (unsigned char)buffer->drawOpts.colorKey))
            {
                // DF_ALWAYS = no depth test
                if(buffer->drawOpts.depthFunc == DF_ALWAYS)
                    gfx_drawPixel(x, y, pixel, buffer);
                else
                    gfx_drawPixelDepth(x, y, lerpInvZ, pixel, buffer);
            }
        }

        startX += dxLeft;
        endX   += dxRight;

        if(finished) break;
    }
}

static void affineTextureMap(const gfx_Bitmap *tex, const gfx_Vertex *v0, const gfx_Vertex *v1, const gfx_Vertex *v2, gfx_drawBuffer *buffer, enum TriangleType type)
{
    double x, y, invDy, dxLeft, dxRight, yDir = 1;
    float duLeft, dvLeft, duRight, dvRight;
    float startU, startV, invDx, du, dv, startX, endX;
    int   useColorKey = buffer->drawOpts.colorKey != -1 ? 1 : 0;
    float texW = tex->width - 1;
    float texH = tex->height - 1;
    int   texArea = texW * texH;
    // variables used only if depth test is enabled
    float invZ0, invZ1, invZ2, invY02 = 1.f;
    int   finished = 0;

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

    for(y = v0->position.y; ; y += yDir)
    {
        float u = startU;
        float v = startV;
        // variables used only if depth test is enabled
        float startInvZ, endInvZ, invLineLength = 0.f;

        if(type == FLAT_BOTTOM && y > v2->position.y)
        {
            // in final iteration draw extra scanline to avoid pixel wide gaps
            u -= duLeft;
            v -= dvLeft;
            startX -= dxLeft;
            endX   -= dxRight;
            finished = 1;
        }
        else if ( type == FLAT_TOP && y < v2->position.y)
            break;

        // skip if no depth testing
        if(buffer->drawOpts.depthFunc != DF_ALWAYS)
        {
            float r1 = (v0->position.y - y) * invY02;
            startInvZ = LERP(invZ0, invZ2, r1);
            endInvZ   = LERP(invZ0, invZ1, r1);

            if(startX != endX)
                invLineLength = 1.f / (endX - startX);
        }

        for(x = startX; x <= endX; ++x)
        {
            // fetch texture data with a texArea modulus for proper effect in case u or v are > 1
            unsigned char pixel = tex->data[((int)u + ((int)v * tex->height)) % texArea];

            if(!useColorKey || (useColorKey && pixel != (unsigned char)buffer->drawOpts.colorKey))
            {
                // DF_ALWAYS = no depth test
                if(buffer->drawOpts.depthFunc == DF_ALWAYS)
                    gfx_drawPixel(x, y, pixel, buffer);
                else
                {
                    float r = (x - startX) * invLineLength;
                    float lerpInvZ = LERP(startInvZ, endInvZ, r);
                    gfx_drawPixelDepth(x, y, lerpInvZ, pixel, buffer);
                }
            }
            u += du;
            v += dv;
        }

        startX += dxLeft;
        endX   += dxRight;
        startU += duLeft;
        startV += dvLeft;

        if(finished) break;
    }
}

