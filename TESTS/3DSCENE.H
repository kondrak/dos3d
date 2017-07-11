#include "src/bitmap.h"
#include "src/camera.h"
#include "src/math.h"
#include "src/timer.h"
#include "src/triangle.h"
#include "src/utils.h"

#define COLOR_MAGENTA 212
#define NUM_WALLS 7
#define NUM_TEXTURES 7

typedef struct SceneQuad
{
    gfx_Triangle tris[2];
} SceneQuad;

typedef struct
{
    gfx_Bitmap textures[NUM_TEXTURES];
    SceneQuad walls[NUM_WALLS];
} Scene;

// helper functions
void setupSceneQuad(SceneQuad *q, int qx, int qy, int qz, int qx2, int qy2, int qz2, float u, float v, gfx_Bitmap *texture);
void drawSceneQuad(const SceneQuad *q, const mth_Matrix4 *mvp, gfx_drawBuffer *buffer);
void setupScene(Scene *s);
void drawScene(const Scene *s, const mth_Matrix4 *mvp, gfx_drawBuffer *buffer);
void freeScene(Scene *s);

// Render a simple Doom scene
void test3DScene()
{
    uint32_t dt, now, last = tmr_getMs();
    Scene scene;
    gfx_Camera cam;
    mth_Matrix4 modelViewProj;
    gfx_drawBuffer buffer;

    ALLOC_DRAWBUFFER(buffer, SCREEN_WIDTH, SCREEN_HEIGHT, DB_COLOR | DB_DEPTH);
    ASSERT(DRAWBUFFER_VALID(buffer, DB_COLOR | DB_DEPTH), "Out of memory!\n");
    buffer.drawOpts.colorKey  = COLOR_MAGENTA;
    buffer.drawOpts.depthFunc = DF_LESS;   

    // setup entire scene
    setupScene(&scene);

    // setup camera
    VEC4(cam.position, 0, -20, 40);
    VEC4(cam.up, 0, 1, 0);
    VEC4(cam.right, 1, 0, 0);
    VEC4(cam.target, 0, 0, -1);

    mth_matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)buffer.width / buffer.height, 0.1f, 500.f);
    mth_matView(&cam.view, &cam.position, &cam.target, &cam.up);
    modelViewProj = mth_matMul(&cam.view, &cam.projection);

    do
    {
        now = tmr_getMs();
        dt  = now - last;

        if(kbd_keyPressed(KEY_T))
            buffer.drawOpts.drawMode = buffer.drawOpts.drawMode == DM_AFFINE ? DM_PERSPECTIVE : DM_AFFINE;

        // clear depth buffer
        gfx_clrBuffer(&buffer, DB_DEPTH);

        // render!
        drawScene(&scene, &modelViewProj, &buffer);

        last = now;
    }  while(!kbd_keyPressed(KEY_ESC));

    FREE_DRAWBUFFER(buffer);
    freeScene(&scene);
}

/* ***** */
void setupSceneQuad(SceneQuad *q, int qx, int qy, int qz, int qx2, int qy2, int qz2, float u, float v, gfx_Bitmap *texture)
{
    q->tris[0].color = 1;
    q->tris[0].texture = texture;
    
    VEC4(q->tris[0].vertices[0].position, qx, qy2, qz);
    q->tris[0].vertices[0].uv.u = 0;
    q->tris[0].vertices[0].uv.v = v;
    VEC4(q->tris[0].vertices[1].position, qx2, qy, qz2);
    q->tris[0].vertices[1].uv.u = u;
    q->tris[0].vertices[1].uv.v = 0;
    VEC4(q->tris[0].vertices[2].position, qx, qy, qz);
    q->tris[0].vertices[2].uv.u = 0;
    q->tris[0].vertices[2].uv.v = 0;

    q->tris[1].color = 1;
    q->tris[1].texture = texture;
    VEC4(q->tris[1].vertices[0].position, qx, qy2, qz);
    q->tris[1].vertices[0].uv.u = 0;
    q->tris[1].vertices[0].uv.v = v;
    VEC4(q->tris[1].vertices[1].position, qx2, qy2, qz2);
    q->tris[1].vertices[1].uv.u = u;
    q->tris[1].vertices[1].uv.v = v;
    VEC4(q->tris[1].vertices[2].position, qx2, qy, qz2);
    q->tris[1].vertices[2].uv.u = u;
    q->tris[1].vertices[2].uv.v = 0; 
}

/* ***** */
void drawSceneQuad(const SceneQuad *q, const mth_Matrix4 *mvp, gfx_drawBuffer *buffer)
{
    gfx_drawTriangle(&q->tris[0], mvp, buffer);
    gfx_drawTriangle(&q->tris[1], mvp, buffer);
}

/* ***** */
void drawScene(const Scene *s, const mth_Matrix4 *mvp, gfx_drawBuffer *buffer)
{
    int w;
    static int bgOffset = 0;
    // background: Doom 1 sky texture, scrolling horizontally
    gfx_drawBitmapOffset(&s->textures[0], 0, 0, bgOffset % s->textures[0].width, 0, buffer);
    bgOffset++;

    // walls and sprites
    for(w = 0; w < NUM_WALLS; w++)
        drawSceneQuad(&s->walls[w], mvp, buffer);

    // push to screen!
    gfx_updateScreen(buffer);
    gfx_vSync();
}

/* ***** */
void setupScene(Scene *s)
{
    int i, k;
    mth_Matrix4 floorModel;
    gfx_Bitmap textureAtlas = gfx_loadBitmap("images/scene.bmp");
    gfx_Bitmap skyTexture   = gfx_bitmapFromAtlas(&textureAtlas, 0, 0, 256, 128);

    // fetch wall textures from atlas
    s->textures[0] = gfx_resizeBitmap(&skyTexture, 320, 128);
    s->textures[1] = gfx_bitmapFromAtlas(&textureAtlas, 0, 256, 128, 128);
    s->textures[2] = gfx_bitmapFromAtlas(&textureAtlas, 0, 128, 128, 128);
    s->textures[3] = gfx_bitmapFromAtlas(&textureAtlas, 128, 128, 128, 128);
    s->textures[4] = gfx_bitmapFromAtlas(&textureAtlas, 128, 256, 128, 128);
    s->textures[5] = gfx_bitmapFromAtlas(&textureAtlas, 0, 384, 128, 128);
    s->textures[6] = gfx_bitmapFromAtlas(&textureAtlas, 128, 384, 128, 128);

    // load the color palette
    gfx_setPalette(textureAtlas.palette);
    // atlas bitmap is no longer needed at this point
    gfx_freeBitmap(&textureAtlas);

    // floor
    setupSceneQuad(&s->walls[0], -140, -48, 0, 140, 48, 0, 3.f, 2.f, &s->textures[2]);

    // setup translation for floor (move down and back a bit after rotation)
    mth_matIdentity(&floorModel);
    floorModel.m[13] = 50.f;
    floorModel.m[14] = -100.f;

    //rotate and shift the floor
    for(k = 0; k < 2; ++k)
    {
        for(i = 0; i < 3; ++i)
        {
            mth_rotateVecAxisAngle(&s->walls[0].tris[k].vertices[i].position, 90.f * M_PI / 180.f, 1.f, 0.f, 0.f);
            s->walls[0].tris[k].vertices[i].position = mth_matMulVec(&floorModel, &s->walls[0].tris[k].vertices[i].position);
        }
    } 

    // right wall
    setupSceneQuad(&s->walls[1],  80, -40, -120, 100, 40, -40, 1.f, 1.f, &s->textures[3]);
    // left wall
    setupSceneQuad(&s->walls[2], -100, -40, -40, -80, 40, -120, 1.f, 1.f, &s->textures[3]);
    // back wall
    setupSceneQuad(&s->walls[3], -80, -120, -120, 80, 40, -120, 1.f, 1.f, &s->textures[1]);
    // baron of hell
    setupSceneQuad(&s->walls[4], 20, -20, -65, 84, 44, -65, 1.f, 1.f, &s->textures[4]);
    // cacodemon
    setupSceneQuad(&s->walls[5], -60, -20, -95, 4, 44, -95, 1.f, 1.f, &s->textures[5]);
    // shotgun guy
    setupSceneQuad(&s->walls[6], -100, -20, -42, -34, 44, -42, 1.f, 1.f, &s->textures[6]);
}

/* ***** */
void freeScene(Scene *s)
{
    int w;
    for(w = 0; w < NUM_TEXTURES; ++w)
        gfx_freeBitmap(&s->textures[w]);
}
