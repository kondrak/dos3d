#include "src/fillers.h"
#include "src/triangle.h"
#include "src/utils.h"

#define VERTEX_SWAP(v1, v2) { gfx_Vertex s = v2; v2 = v1; v1 = s; }

extern gfx_drawBuffer VGA_BUFFER;

// internal: perform triangle rendering based on its type
static void drawTriangleType(const gfx_Triangle *t, gfx_drawBuffer *buffer, enum TriangleType type);

// determine if triangle is degenerate
#define DEGENERATE(v0, v1, v2) ( (v0.position.x == v1.position.x && v0.position.x == v2.position.x) || \
                                 (v0.position.y == v1.position.y && v0.position.y == v2.position.y) )

// determine if either coordinate of all vertices is offscreen
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
void gfx_drawTriangle(const gfx_Triangle *t, const mth_Matrix4 *matrix, gfx_drawBuffer *target)
{
    gfx_drawBuffer *buffer = target ? target : &VGA_BUFFER;
    int bufferHalfWidth  = buffer->width  >> 1;
    int bufferHalfHeight = buffer->height >> 1;
    gfx_Vertex v0, v1, v2;
    gfx_Triangle sortedTriangle = *t;

    // DF_NEVER - don't draw anything, abort
    if(buffer->drawOpts.depthFunc == DF_NEVER)
        return;

    v0 = t->vertices[0];
    v1 = t->vertices[1];
    v2 = t->vertices[2];

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

        // face culled? abort!
        if(buffer->drawOpts.cullMode == FC_BACK && dp >= 0)
            return;
        else if(buffer->drawOpts.cullMode == FC_FRONT && dp < 0)
            return;
    }

    // transform x and y of each vertex to screen coordinates
    v0.position.x = (v0.position.x * buffer->width)  / (2.0 * v0.position.w) + bufferHalfWidth;
    v0.position.y = (v0.position.y * buffer->height) / (2.0 * v0.position.w) + bufferHalfHeight;
    v1.position.x = (v1.position.x * buffer->width)  / (2.0 * v1.position.w) + bufferHalfWidth;
    v1.position.y = (v1.position.y * buffer->height) / (2.0 * v1.position.w) + bufferHalfHeight;
    v2.position.x = (v2.position.x * buffer->width)  / (2.0 * v2.position.w) + bufferHalfWidth;
    v2.position.y = (v2.position.y * buffer->height) / (2.0 * v2.position.w) + bufferHalfHeight;

    // sort vertices so that v0 is topmost, then v2, then v1
    if(v2.position.y > v1.position.y)
        VERTEX_SWAP(v1, v2)

    if(v0.position.y > v1.position.y)
        VERTEX_SWAP(v0, v1)

    if(v0.position.y > v2.position.y)
        VERTEX_SWAP(v0, v2)

    // discard degenerate triangle
    if(DEGENERATE(v0, v1, v2))
        return;

    // rendering wireframe?
    if(buffer->drawOpts.drawMode & DM_WIREFRAME)
    {
        sortedTriangle.vertices[0] = v0;
        sortedTriangle.vertices[1] = v1;
        sortedTriangle.vertices[2] = v2;
        gfx_wireFrame(&sortedTriangle, buffer);
        return;
    }

    // handle 2 basic cases of flat bottom and flat top triangles
    if(v1.position.y == v2.position.y)
    {
        sortedTriangle.vertices[0] = v0;
        sortedTriangle.vertices[1] = v1;
        sortedTriangle.vertices[2] = v2;
        drawTriangleType(&sortedTriangle, buffer, FLAT_BOTTOM);
    }
    else if(v0.position.y == v1.position.y)
    {
        sortedTriangle.vertices[0] = v2;
        sortedTriangle.vertices[1] = v1;
        sortedTriangle.vertices[2] = v0;
        drawTriangleType(&sortedTriangle, buffer, FLAT_TOP);
    }
    else
    {
        // "Non-trivial" triangles will be broken down into a composition of flat bottom and flat top triangles.
        // For this, we need to calculate a new vertex, v3 and interpolate its UV coordinates.
        gfx_Vertex v3;
        mth_Vector4 diff, diff2;
        double ratioU = 1, ratioV = 1;

        // calculate v3.x with Intercept Theorem, y is the same as v2
        v3.position.x = v0.position.x + (v1.position.x - v0.position.x) * (v2.position.y - v0.position.y) / (v1.position.y - v0.position.y);
        v3.position.y = v2.position.y;
        // setting this to avoid undefined behavior when subtracting vectors
        v3.position.z = 0;

        diff  = mth_vecSub(&v1.position, &v0.position);
        diff2 = mth_vecSub(&v3.position, &v0.position);

        if(diff.x != 0)
            ratioU = diff2.x / diff.x;

        if(diff.y != 0)
            ratioV = diff2.y / diff.y;

        // lerp 1/Z and UV for v3. For perspective texture mapping calculate u/z, v/z, for affine skip unnecessary divisions;
        // perform this step for affine texture mapping only if depth testing is enabled, since then correct Z is needed for v3!
        if(buffer->drawOpts.drawMode == DM_PERSPECTIVE || buffer->drawOpts.depthFunc != DF_ALWAYS)
        {
            float invV0Z = 1.f/v0.position.z;
            float invV1Z = 1.f/v1.position.z;

            // get v3.z value by interpolating 1/z (it's lerp-able)
            if(v0.position.x - v1.position.x)
                v3.position.z = 1.0 / LERP(invV1Z, invV0Z, (v3.position.x - v1.position.x) / (v0.position.x - v1.position.x));
            else
                v3.position.z = v0.position.z;

            // skip this step for affine texture mapping - distortion will be too high if UVs are lerped with 1/Z
            if(buffer->drawOpts.drawMode == DM_PERSPECTIVE)
            {
                v3.uv.u = v3.position.z * LERP(v0.uv.u * invV0Z, v1.uv.u * invV1Z, ratioU);
                v3.uv.v = v3.position.z * LERP(v0.uv.v * invV0Z, v1.uv.v * invV1Z, ratioV);
            }
        }

        // for affine texture mapping, approximating v3.uv without taking Z into account gives better results
        if(buffer->drawOpts.drawMode == DM_AFFINE)
        {
            v3.uv.u = LERP(v0.uv.u, v1.uv.u, ratioU);
            v3.uv.v = LERP(v0.uv.v, v1.uv.v, ratioV);
        }

        // this swap is done to maintain consistent renderer behavior
        if(v3.position.x < v2.position.x)
            VERTEX_SWAP(v3, v2)

        // draw the composition of both triangles to form the desired shape
        if(!DEGENERATE(v0, v3, v2))
        {
            sortedTriangle.vertices[0] = v0;
            sortedTriangle.vertices[1] = v3;
            sortedTriangle.vertices[2] = v2;
            drawTriangleType(&sortedTriangle, buffer, FLAT_BOTTOM);
        }

        if(!DEGENERATE(v1, v3, v2))
        {
            sortedTriangle.vertices[0] = v1;
            sortedTriangle.vertices[1] = v3;
            sortedTriangle.vertices[2] = v2;
            drawTriangleType(&sortedTriangle, buffer, FLAT_TOP);
        }
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
static void drawTriangleType(const gfx_Triangle *t, gfx_drawBuffer *target, enum TriangleType type)
{
    if(!t->texture || target->drawOpts.drawMode & DM_FLAT)
        gfx_flatFill(t, target, type);
    else
    {
        if(target->drawOpts.drawMode & DM_AFFINE)
            gfx_affineTextureMap(t, target, type);
        else
            gfx_perspectiveTextureMap(t, target, type);
    }
}
