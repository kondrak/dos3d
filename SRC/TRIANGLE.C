#include "src/triangle.h"

// simplest case: will plot either a flat bottom or flat top triangles
enum TriangleType
{
    FLAT_BOTTOM,
    FLAT_TOP
};

// internal triangle renderer based on triangle type
void drawTriangleType(const Triangle *t, const Vertex *v0, const Vertex *v1, const Vertex *v2, unsigned char *buffer, enum TriangleType type);

// draw the triangle!
void drawTriangle(const Triangle *t, unsigned char *buffer)
{
    const Vertex *v0, *v1, *v2;

    v0 = &t->vertices[0];
    v1 = &t->vertices[1];
    v2 = &t->vertices[2];

    // sort vertices so that v0 is topmost, then v1, then v2
    if(v0->position.y > v1->position.y)
    {
        v0 = &t->vertices[1];
        v1 = &t->vertices[0];
    }

    if(v0->position.y > v2->position.y)
    {
        v2 = v0;
        v0 = &t->vertices[2];
    }

    // degenerate triangle?
    //if((int)v0->position.y == (int)v2->position.y)
    //    return;

    if(v1->position.y == v2->position.y)
        drawTriangleType(t, v0, v1, v2, buffer, FLAT_BOTTOM);
    else if(v0->position.y == v1->position.y)
        drawTriangleType(t, v2, v1, v0, buffer, FLAT_TOP);
    else
    {
        Vertex v3;

        if(v2->position.y > v1->position.y)
        {
            const Vertex *tmp = v2;
            v2 = v1;
            v1 = tmp;
        }

        v3.position.x = v0->position.x + ((float)(v2->position.y - v0->position.y) / (float)(v1->position.y - v0->position.y)) * (v1->position.x - v0->position.x);
        v3.position.y = v2->position.y;
        v3.position.z = v2->position.z;
        v3.uv.u = v1->uv.u * 0.5;
        v3.uv.v = v1->uv.v * 0.5;

        // more degenerate triangle tests
        //if((int)v0->position.y == (int)v3.position.y)
        //    return;

        //if((int)v0->position.y == (int)v2->position.y)
        //    return;

        drawTriangleType(t, v0, &v3, v2, buffer, FLAT_BOTTOM);

        //if((int)v1->position.y == (int)v3.position.y)
        //    return;

        //if((int)v1->position.y == (int)v2->position.y)
        //    return;

        drawTriangleType(t, v1, &v3, v2, buffer, FLAT_TOP);
    }
}


/*
* Render counter clockwise (v0->v1->v2) for wanted effect.
* v0         v0----v1
* |\         |     /
* | \        |    /
* |  \       |   /
* |   \      |  /
* |    \     | /
* |     \    |/
* v2-----v1  v2
*/
void drawTriangleType(const Triangle *t, const Vertex *v0, const Vertex *v1, const Vertex *v2, unsigned char *buffer, enum TriangleType type)
{
    double invDy, dxLeft, dxRight, xLeft, xRight;
    double x, y, yDir = 1;
    
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
            // to avoid pixel wide gaps, render extra line at the junction between two final points
            if(type == FLAT_TOP && y < v2->position.y)
            {
                //drawLine(xLeft-dxLeft, y, xRight-dxRight, y, t->color, buffer);
                break;
            }
            else if(type == FLAT_BOTTOM && y > v2->position.y)
            {
                drawLine(xLeft-dxLeft, y, xRight-dxRight, y, t->color, buffer);
                break;
            }

            drawLine(xLeft, y, xRight, y, t->color, buffer);
            xLeft += dxLeft;
            xRight += dxRight;

        }
    }
    else
    {
        float texW = t->texture->width - 1;
        float texH = t->texture->height - 1;
        float duLeft =  texW * (v2->uv.u - v0->uv.u) * invDy;
        float dvLeft =  texH * (v2->uv.v - v0->uv.v) * invDy;
        float duRight = texW * (v1->uv.u - v0->uv.u) * invDy;
        float dvRight = texH * (v1->uv.v - v0->uv.v) * invDy;

        float uLeft = texW * v0->uv.u;
        float uRight = uLeft;
        float vLeft = texH * v0->uv.v;
        float vRight = vLeft;
        int startY = v0->position.y;
        int endY = v2->position.y;

        setPalette(t->texture->palette);
        
        if(type == FLAT_BOTTOM)
            y = startY;
        else
        {
            startY = v2->position.y;
            endY   = v0->position.y;
            y = endY;
        }

        for(; ; y += yDir)
        {
            int startX = xLeft;
            int endX  = xRight;
            float u = uLeft;
            float v = vLeft;
            float du, dv;
            float dx = endX - startX;

           if(type == FLAT_BOTTOM && y > v2->position.y)
                break;
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
                byte pixel = t->texture->data[(int)u + ((int)v << 6)];
                drawPixel(x, y, pixel, buffer);
                u += du;
                v += dv;
            }

            xLeft += dxLeft;
            xRight += dxRight;
            uLeft += duLeft;
            uRight += duRight;
            vLeft += dvLeft;
            vRight += dvRight;
        }
    }
}
