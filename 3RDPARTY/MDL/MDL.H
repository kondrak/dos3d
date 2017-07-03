#ifndef MDL_H
#define MDL_H
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

#include <stdint.h>
 
#ifdef __cplusplus
extern "C" {
#endif

    /* Vector */
    typedef float vec3_t[3];
    typedef int GLuint;
    typedef float GLfloat;

    /* MDL header */
    typedef struct 
    {
        int ident;             /* magic number: "IDPO" */
        int version;           /* version: 6 */

        vec3_t scale;          /* scale factor */
        vec3_t translate;      /* translation vector */
        float  boundingradius;
        vec3_t eyeposition;    /* eyes' position */

        int num_skins;         /* number of textures */
        int skinwidth;         /* texture width */
        int skinheight;        /* texture height */

        int num_verts;         /* number of vertices */
        int num_tris;          /* number of triangles */
        int num_frames;        /* number of frames */

        int   synctype;        /* 0 = synchron, 1 = random */
        int   flags;           /* state flag */
        float size;
    } mdl_header_t;

    /* Skin */
    typedef struct
    {
        int group;      /* 0 = single, 1 = group */
        uint8_t *data;  /* texture data */
    } mdl_skin_t;

    /* Texture coords */
    typedef struct
    {
        int onseam;
        int s;
        int t;
    } mdl_texcoord_t;

    /* Triangle info */
    typedef struct
    {
        int facesfront;  /* 0 = backface, 1 = frontface */
        int vertex[3];   /* vertex indices */
    } mdl_triangle_t;

    /* Compressed vertex */
    typedef struct
    {
        unsigned char v[3];
        unsigned char normalIndex;
    } mdl_vertex_t;

    /* Simple frame */
    typedef struct
    {
        mdl_vertex_t bboxmin; /* bouding box min */
        mdl_vertex_t bboxmax; /* bouding box max */
        char name[16];
        mdl_vertex_t *verts;  /* vertex list of the frame */
    } mdl_simpleframe_t;

    /* Model frame */
    typedef struct
    {
        int type;                 /* 0 = simple, !0 = group */
        mdl_simpleframe_t frame;  /* this program can't read models
                                     composed of group frames! */
    } mdl_frame_t;

    /* MDL model structure */
    typedef struct
    {
        mdl_header_t header;

        mdl_skin_t     *skins;
        mdl_texcoord_t *texcoords;
        mdl_triangle_t *triangles;
        mdl_frame_t    *frames;

        GLuint *tex_id;
        int iskin;
    } mdl_model_t;

    // Make a texture given a skin index 'n'.
    GLuint MakeTextureFromSkin(int n, const mdl_model_t *mdl);
    int ReadMDLModel(const char *filename, mdl_model_t *mdl);
    void FreeModel (mdl_model_t *mdl);
    // Render the model at frame n.
    void RenderFrame(int n, const mdl_model_t *mdl);

    /**
    * Render the model with interpolation between frame n and n+1.
    * interp is the interpolation percent. (from 0.0 to 1.0)
    */
    void RenderFrameItp(int n, float interp, const mdl_model_t *mdl);

    /**
    * Calculate the current frame in animation beginning at frame
    * 'start' and ending at frame 'end', given interpolation percent.
    * interp will be reseted to 0.0 if the next frame is reached.
    */
    void Animate(int start, int end, int *frame, float *interp);

    void init(const char *filename);
    void cleanup();
    void reshape(int w, int h);
    void display();

#ifdef __cplusplus
}
#endif
#endif
