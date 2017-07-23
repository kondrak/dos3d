/*
 * mdl.c -- mdl model loader
 * last modification: mar. 21, 2015
 *
 * Copyright (c) 2005-2015 David HENRY
 * Open Watcom update (c) 2017 Krzysztof Kondrak
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "mdl.h"
#include "src/triangle.h"
#include "src/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Table of precalculated normals (currently unused) */
//#include "anorms.h"
/* Quake Palette */
#include "colormap.h"

// internal: create a bitmap texture using n-th skin's image data
static gfx_Bitmap bitmapFromSkin(int n, const mdl_model_t *mdl)
{
    gfx_Bitmap texture;
    texture.width  = mdl->header.skinwidth;
    texture.height = mdl->header.skinheight;
    texture.data   = (uint8_t *)malloc(sizeof(uint8_t) * mdl->header.skinwidth * mdl->header.skinheight);
    ASSERT(texture.data, "Error allocating memory for MDL texture!\n");

    memcpy(texture.palette, colormap, sizeof(uint8_t)*256*3);
    memcpy(texture.data, mdl->skins[n].data, sizeof(uint8_t) * mdl->header.skinwidth * mdl->header.skinheight);

    // renderer currently supports only square textures, so resize it properly
    texture = gfx_resizeBitmap(&texture, MAX(mdl->header.skinwidth, mdl->header.skinheight), 
                                         MAX(mdl->header.skinwidth, mdl->header.skinheight));
    return texture;
}

/**
 * Load an MDL model from file.
 *
 * Note: MDL format stores model's data in little-endian ordering.  On
 * big-endian machines, you'll have to perform proper conversions.
 */
void mdl_load(const char *filename, mdl_model_t *mdl)
{
    FILE *fp;
    int i;

    fp = fopen(filename, "rb");
    ASSERT(fp, "error: couldn't open \"%s\"!\n", filename);

    /* Read header */
    fread(&mdl->header, 1, sizeof(mdl_header_t), fp);

    if((mdl->header.ident != 1330660425) || (mdl->header.version != 6))
    {
        /* Error! */
        fclose(fp);
        ASSERT(0, "Error: bad version or identifier\n");
    }

    /* Memory allocations */
    mdl->skins = (mdl_skin_t *)malloc(sizeof(mdl_skin_t) * mdl->header.num_skins);
    ASSERT(mdl->skins, "Error allocating memory for MDL skins!\n");
    mdl->texcoords = (mdl_texcoord_t *)malloc(sizeof(mdl_texcoord_t) * mdl->header.num_verts);
    ASSERT(mdl->texcoords, "Error allocating memory for MDL texcoords!\n");
    mdl->triangles = (mdl_triangle_t *)malloc(sizeof(mdl_triangle_t) * mdl->header.num_tris);
    ASSERT(mdl->triangles, "Error allocating memory for MDL triangles!\n");
    mdl->frames = (mdl_frame_t *)malloc(sizeof(mdl_frame_t) * mdl->header.num_frames);
    ASSERT(mdl->frames, "Error allocating memory for MDL frames!\n");
    mdl->skinTextures = (gfx_Bitmap *)malloc(sizeof(gfx_Bitmap) * mdl->header.num_skins);
    ASSERT(mdl->skinTextures, "Error allocating memory for MDL skin textures!\n");
    mdl->iskin = 0;

    /* Read texture data */
    for(i = 0; i < mdl->header.num_skins; ++i)
    {
        mdl->skins[i].data = (uint8_t *)malloc(sizeof(uint8_t) * mdl->header.skinwidth * mdl->header.skinheight);
        ASSERT(mdl->skins[i].data, "Error allocating memory for MDL skin data!\n");

        fread(&mdl->skins[i].group, sizeof(int), 1, fp);
        fread(mdl->skins[i].data, sizeof(uint8_t), mdl->header.skinwidth * mdl->header.skinheight, fp);

        mdl->skinTextures[i] = bitmapFromSkin(i, mdl);

        free(mdl->skins[i].data);
        mdl->skins[i].data = NULL;
    }

    fread(mdl->texcoords, sizeof(mdl_texcoord_t), mdl->header.num_verts, fp);
    fread(mdl->triangles, sizeof(mdl_triangle_t), mdl->header.num_tris,  fp);

    /* Read frames */
    for(i = 0; i < mdl->header.num_frames; ++i)
    {
        /* Memory allocation for vertices of this frame */
        mdl->frames[i].frame.verts = (mdl_vertex_t *)malloc(sizeof(mdl_vertex_t) * mdl->header.num_verts);
        ASSERT(mdl->frames[i].frame.verts, "Error allocating memory for MDL frame verts!\n");

        /* Read frame data */
        fread(&mdl->frames[i].type, sizeof(int), 1, fp);
        fread(&mdl->frames[i].frame.bboxmin, sizeof(mdl_vertex_t), 1, fp);
        fread(&mdl->frames[i].frame.bboxmax, sizeof(mdl_vertex_t), 1, fp);
        fread(mdl->frames[i].frame.name, sizeof(char), 16, fp);
        fread(mdl->frames[i].frame.verts, sizeof(mdl_vertex_t), mdl->header.num_verts, fp);
    }

    fclose(fp);
}

/* ***** */
void mdl_free(mdl_model_t *mdl)
{
    int i;

    if(mdl->skins)
    {
        free(mdl->skins);
        mdl->skins = NULL;
    }

    if(mdl->texcoords)
    {
        free(mdl->texcoords);
        mdl->texcoords = NULL;
    }

    if(mdl->triangles)
    {
        free(mdl->triangles);
        mdl->triangles = NULL;
    }

    if(mdl->skinTextures)
    {
        int i = 0;
        for(i = 0; i < mdl->header.num_skins; ++i)
            gfx_freeBitmap(&mdl->skinTextures[i]);

        free(mdl->skinTextures);
        mdl->skinTextures = NULL;
    }

    if(mdl->frames)
    {
        for(i = 0; i < mdl->header.num_frames; ++i)
        {
            free(mdl->frames[i].frame.verts);
            mdl->frames[i].frame.verts = NULL;
        }

        free(mdl->frames);
        mdl->frames = NULL;
    }
}

/* ***** */
void mdl_renderFrame(int n, const mdl_model_t *mdl, const mth_Matrix4 *matrix, gfx_drawBuffer *target)
{
    int i, j;
    float s, t;
    mdl_vertex_t *pvert;

    /* Check if n is in a valid range */
    if((n < 0) || (n > mdl->header.num_frames - 1))
        return;

    /* Draw all model triangles */
    for(i = 0; i < mdl->header.num_tris; ++i)
    {
        gfx_Triangle mdlTriangle;
        mdlTriangle.color = 12;
        mdlTriangle.texture = &mdl->skinTextures[mdl->iskin];

        /* Draw each vertex */
        for(j = 0; j < 3; ++j)
        {
            pvert = &mdl->frames[n].frame.verts[mdl->triangles[i].vertex[j]];

            /* Compute texture coordinates */
            s = (float)mdl->texcoords[mdl->triangles[i].vertex[j]].s;
            t = (float)mdl->texcoords[mdl->triangles[i].vertex[j]].t;

            if(!mdl->triangles[i].facesfront &&
            mdl->texcoords[mdl->triangles[i].vertex[j]].onseam)
            {
                s += mdl->header.skinwidth * 0.5f; /* Backface */
            }

            /* Scale s and t to range from 0.0 to 1.0 */
            mdlTriangle.vertices[j].uv.u = (s + 0.5) / mdl->header.skinwidth;
            mdlTriangle.vertices[j].uv.v = (t + 0.5) / mdl->header.skinheight;

            /* Calculate real vertex position */
            VEC4(mdlTriangle.vertices[j].position,
                 mdl->header.scale[0] * pvert->v[0] + mdl->header.translate[0],
                 mdl->header.scale[1] * pvert->v[1] + mdl->header.translate[1],
                 mdl->header.scale[2] * pvert->v[2] + mdl->header.translate[2]);
        }

        gfx_drawTriangle(&mdlTriangle, matrix, target);
    }
}

/* ***** */
void mdl_renderFrameLerp(int n, float r, const mdl_model_t *mdl, const mth_Matrix4 *matrix, gfx_drawBuffer *target)
{
    int i, j;
    float s, t;
    mdl_vertex_t *pvert1, *pvert2;

    /* Check if n is in a valid range */
    if((n < 0) || (n > mdl->header.num_frames))
        return;

    /* Draw all model triangles */
    for(i = 0; i < mdl->header.num_tris; ++i)
    {
        gfx_Triangle mdlTriangle;
        mdlTriangle.color = 12;
        mdlTriangle.texture = &mdl->skinTextures[mdl->iskin];

        /* Draw each vertex */
        for(j = 0; j < 3; ++j)
        {
            pvert1 = &mdl->frames[n].frame.verts[mdl->triangles[i].vertex[j]];
            pvert2 = &mdl->frames[n + 1].frame.verts[mdl->triangles[i].vertex[j]];

            /* Compute texture coordinates */
            s = (float)mdl->texcoords[mdl->triangles[i].vertex[j]].s;
            t = (float)mdl->texcoords[mdl->triangles[i].vertex[j]].t;

            if(!mdl->triangles[i].facesfront &&
            mdl->texcoords[mdl->triangles[i].vertex[j]].onseam)
            {
                s += mdl->header.skinwidth * 0.5f; /* Backface */
            }

            /* Scale s and t to range from 0.0 to 1.0 */
            mdlTriangle.vertices[j].uv.u = (s + 0.5) / mdl->header.skinwidth;
            mdlTriangle.vertices[j].uv.v = (t + 0.5) / mdl->header.skinheight;

            /* Interpolate vertices */
            VEC4(mdlTriangle.vertices[j].position,
                 mdl->header.scale[0] * (pvert1->v[0] + r * (pvert2->v[0] - pvert1->v[0])) + mdl->header.translate[0],
                 mdl->header.scale[1] * (pvert1->v[1] + r * (pvert2->v[1] - pvert1->v[1])) + mdl->header.translate[1],
                 mdl->header.scale[2] * (pvert1->v[2] + r * (pvert2->v[2] - pvert1->v[2])) + mdl->header.translate[2]);
        }

        gfx_drawTriangle(&mdlTriangle, matrix, target);
    }
}

/* ***** */
void mdl_animate(int start, int end, int *frame, float *r)
{
    if((*frame < start) || (*frame > end))
        *frame = start;

    if(*r >= 1.0f)
    {
        /* Move to next frame */
        *r = 0.0f;
        (*frame)++;

        if(*frame >= end)
            *frame = start;
    }
}
