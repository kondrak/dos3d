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
    
    // sort vertices here
    
    if(v1->position.y == v2->position.y)
        drawTriangleType(t, v0, v1, v2, buffer, FLAT_BOTTOM);
    
    if(v0->position.y == v1->position.y)
        drawTriangleType(t, v2, v1, v0, buffer, FLAT_TOP);
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
    float invDy, dxLeft, dxRight, xLeft, xRight;
    int x, y, yDir = 1;
    
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
            drawLine(xLeft, y, xRight, y, t->color, buffer);
            xLeft += dxLeft;
            xRight += dxRight;
            
            if(type == FLAT_BOTTOM && y > v2->position.y)
                break;
            else if(type == FLAT_TOP && y < v2->position.y)
                break;
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
            
           if(type == FLAT_BOTTOM && y > endY)
                break;
            else if ( y < startY)
                break;
        }
    }
}
