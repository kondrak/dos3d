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

/* Table of precalculated normals */
#include "anorms.h"
/* Palette */
#include "colormap.h"

/**
 * Make a texture given a skin index 'n'.
 */
gfx_Bitmap MakeTextureFromSkin(int n, const mdl_model_t *mdl)
{
    gfx_Bitmap texture;
    texture.width  = mdl->header.skinwidth;
    texture.height = mdl->header.skinheight;
    texture.data   = (uint8_t *)malloc(sizeof(uint8_t) * mdl->header.skinwidth * mdl->header.skinheight);

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
    mdl->texcoords = (mdl_texcoord_t *)malloc(sizeof(mdl_texcoord_t) * mdl->header.num_verts);
    mdl->triangles = (mdl_triangle_t *)malloc(sizeof(mdl_triangle_t) * mdl->header.num_tris);
    mdl->frames = (mdl_frame_t *)malloc(sizeof(mdl_frame_t) * mdl->header.num_frames);
    mdl->skinTextures = (gfx_Bitmap *)malloc(sizeof(gfx_Bitmap) * mdl->header.num_skins);
    mdl->iskin = 0;

    /* Read texture data */
    for(i = 0; i < mdl->header.num_skins; ++i)
    {
        mdl->skins[i].data = (uint8_t *)malloc(sizeof(uint8_t) * mdl->header.skinwidth * mdl->header.skinheight);

        fread(&mdl->skins[i].group, sizeof(int), 1, fp);
        fread(mdl->skins[i].data, sizeof(uint8_t), mdl->header.skinwidth * mdl->header.skinheight, fp);

        mdl->skinTextures[i] = MakeTextureFromSkin(i, mdl);

        free(mdl->skins[i].data);
        mdl->skins[i].data = NULL;
    }

    fread(mdl->texcoords, sizeof(mdl_texcoord_t), mdl->header.num_verts, fp);
    fread(mdl->triangles, sizeof(mdl_triangle_t), mdl->header.num_tris, fp);

    /* Read frames */
    for(i = 0; i < mdl->header.num_frames; ++i)
    {
        /* Memory allocation for vertices of this frame */
        mdl->frames[i].frame.verts = (mdl_vertex_t *)malloc(sizeof(mdl_vertex_t) * mdl->header.num_verts);

        /* Read frame data */
        fread(&mdl->frames[i].type, sizeof(int), 1, fp);
        fread(&mdl->frames[i].frame.bboxmin, sizeof(mdl_vertex_t), 1, fp);
        fread(&mdl->frames[i].frame.bboxmax, sizeof(mdl_vertex_t), 1, fp);
        fread(mdl->frames[i].frame.name, sizeof(char), 16, fp);
        fread(mdl->frames[i].frame.verts, sizeof(mdl_vertex_t), mdl->header.num_verts, fp);
    }

    fclose(fp);
}

/**
 * Free resources allocated for the model.
 */
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

/**
 * Render the model at frame n.
 */
void mdl_renderFrame(int n, const mdl_model_t *mdl, const mth_Matrix4 *matrix, gfx_drawBuffer *buffer)
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
            mdlTriangle.vertices[j].position.x = (mdl->header.scale[0] * pvert->v[0]) + mdl->header.translate[0];
            mdlTriangle.vertices[j].position.y = (mdl->header.scale[1] * pvert->v[1]) + mdl->header.translate[1];
            mdlTriangle.vertices[j].position.z = (mdl->header.scale[2] * pvert->v[2]) + mdl->header.translate[2];
            mdlTriangle.vertices[j].position.w = 1.0;
        }

        gfx_drawTriangle(&mdlTriangle, matrix, buffer);
    }
}

/**
 * Render the model with interpolation between frame n and n+1.
 * interp is the interpolation percent. (from 0.0 to 1.0)
 */
void mdl_renderFrameLerp(int n, float interp, const mdl_model_t *mdl, const mth_Matrix4 *matrix, gfx_drawBuffer *buffer)
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
            mdlTriangle.vertices[j].position.x = mdl->header.scale[0] * (pvert1->v[0] + interp
                                                * (pvert2->v[0] - pvert1->v[0])) + mdl->header.translate[0];
            mdlTriangle.vertices[j].position.y = mdl->header.scale[1] * (pvert1->v[1] + interp
                                                * (pvert2->v[1] - pvert1->v[1])) + mdl->header.translate[1];
            mdlTriangle.vertices[j].position.z = mdl->header.scale[2] * (pvert1->v[2] + interp
                                                * (pvert2->v[2] - pvert1->v[2])) + mdl->header.translate[2];
            mdlTriangle.vertices[j].position.w = 1.0;
        }

        gfx_drawTriangle(&mdlTriangle, matrix, buffer);
    }
}

/**
 * Calculate the current frame in animation beginning at frame
 * 'start' and ending at frame 'end', given interpolation percent.
 * interp will be reseted to 0.0 if the next frame is reached.
 */
void mdl_animate(int start, int end, int *frame, float *interp)
{
    if((*frame < start) || (*frame > end))
        *frame = start;

    if(*interp >= 1.0f)
    {
        /* Move to next frame */
        *interp = 0.0f;
        (*frame)++;

        if(*frame >= end)
            *frame = start;
    }
}
