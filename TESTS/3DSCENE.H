#include "src/bitmap.h"
#include "src/camera.h"
#include "src/math.h"
#include "src/timer.h"
#include "src/triangle.h"

typedef struct SceneQuad SceneQuad;

typedef struct SceneQuad
{
    gfx_Triangle tris[2];
} SceneQuad;

#define COLOR_MAGENTA 212
#define NUM_WALLS 7
#define NUM_TEXTURES 7

typedef struct
{
    gfx_Bitmap textures[NUM_TEXTURES];
    SceneQuad walls[NUM_WALLS];
} Scene;

// helper functions
void setupSceneQuad(SceneQuad *q, int qx, int qy, int qz, int qx2, int qy2, int qz2, float u, float v, gfx_Bitmap *texture);
void drawSceneQuad(const SceneQuad *q, const mth_Matrix4 *mvp, const gfx_drawOptions *drawOpts, gfx_drawBuffer *buffer);
void setupScene(Scene *s);
void drawScene(const Scene *s, const mth_Matrix4 *mvp, const gfx_drawOptions *drawOpts, gfx_drawBuffer *buffer);
void freeScene(Scene *s);

// Render a simple scene
void test3DScene()
{
    unsigned long int now, last = 0;
    const unsigned short *keysPressed;
    Scene scene;
    int texMapFlipped = 0;
    gfx_Camera cam;
    gfx_drawOptions drawOpts;
    mth_Matrix4 model;
    mth_Matrix4 modelViewProj;
    gfx_drawBuffer buffer;

    DRAWOPTS_DEFAULT(drawOpts);
    drawOpts.colorKey = COLOR_MAGENTA;
    
    buffer.colorBuffer = (unsigned char *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT);

    if(!buffer.colorBuffer)
    {
        printf("Out of memory!");
        exit(1);
    }

    tmr_start();

    setupScene(&scene);
    mth_matIdentity(&model);
    mth_matIdentity(&modelViewProj);

    // setup camera
    cam.position.x = 0;
    cam.position.y = -40.f;
    cam.position.z = 40.f;
    cam.position.w = 1.f;

    cam.up.x = 0.f;
    cam.up.y = 1.f;
    cam.up.z = 0.f;
    cam.up.w = 1.f;

    cam.right.x = 1.f;
    cam.right.y = 0.f;
    cam.right.z = 0.f;
    cam.right.w = 1.f;

    cam.target.x = 0.f;
    cam.target.y = 0.f;
    cam.target.z = -1.f;
    cam.target.w = 1.f;

    // translate entire cube a bit
    model.m[13] = -20.f;

    keysPressed = kbd_getInput();
    mth_matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 500.f);

    while(!keysPressed[KEY_ESC])
    {
        float dt;
        now = tmr_getMs();
        dt = (float)(now - last);

        if(keysPressed[KEY_G] && !texMapFlipped)
        {
            drawOpts.texMapMode = drawOpts.texMapMode == TM_AFFINE ? TM_PERSPECTIVE : TM_AFFINE;
            texMapFlipped = 1;
        }
        else if(!keysPressed[KEY_G]) texMapFlipped = 0;

        mth_matView(&cam.view, &cam.position, &cam.target, &cam.up);
        modelViewProj = mth_matMul(&cam.view, &cam.projection);
        modelViewProj = mth_matMul(&model, &modelViewProj);

        // render!
        drawScene(&scene, &modelViewProj, &drawOpts, &buffer);

        keysPressed = kbd_getInput();
        last = now;
    }

    tmr_finish();
    free(buffer.colorBuffer);
    freeScene(&scene);
}


void setupSceneQuad(SceneQuad *q, int qx, int qy, int qz, int qx2, int qy2, int qz2, float u, float v, gfx_Bitmap *texture)
{
    q->tris[0].color = 1;
    q->tris[0].texture = texture;
    q->tris[0].vertices[0].position.x = qx;
    q->tris[0].vertices[0].position.y = qy2;
    q->tris[0].vertices[0].position.z = qz;
    q->tris[0].vertices[0].position.w = 1.f;
    q->tris[0].vertices[0].uv.u = 0;
    q->tris[0].vertices[0].uv.v = v;
    q->tris[0].vertices[1].position.x = qx2;
    q->tris[0].vertices[1].position.y = qy;
    q->tris[0].vertices[1].position.z = qz2;
    q->tris[0].vertices[1].position.w = 1.f;
    q->tris[0].vertices[1].uv.u = u;
    q->tris[0].vertices[1].uv.v = 0;
    q->tris[0].vertices[2].position.x = qx;
    q->tris[0].vertices[2].position.y = qy;
    q->tris[0].vertices[2].position.z = qz;
    q->tris[0].vertices[2].position.w = 1.f;
    q->tris[0].vertices[2].uv.u = 0;
    q->tris[0].vertices[2].uv.v = 0;

    q->tris[1].color = 1;
    q->tris[1].texture = texture;
    q->tris[1].vertices[0].position.x = qx;
    q->tris[1].vertices[0].position.y = qy2;
    q->tris[1].vertices[0].position.z = qz;
    q->tris[1].vertices[0].position.w = 1.f;
    q->tris[1].vertices[0].uv.u = 0;
    q->tris[1].vertices[0].uv.v = v;
    q->tris[1].vertices[1].position.x = qx2;
    q->tris[1].vertices[1].position.y = qy2;
    q->tris[1].vertices[1].position.z = qz2;
    q->tris[1].vertices[1].position.w = 1.f;
    q->tris[1].vertices[1].uv.u = u;
    q->tris[1].vertices[1].uv.v = v;
    q->tris[1].vertices[2].position.x = qx2;
    q->tris[1].vertices[2].position.y = qy;
    q->tris[1].vertices[2].position.z = qz2;
    q->tris[1].vertices[2].position.w = 1.f;
    q->tris[1].vertices[2].uv.u = u;
    q->tris[1].vertices[2].uv.v = 0; 
}

void drawSceneQuad(const SceneQuad *q, const mth_Matrix4 *mvp, const gfx_drawOptions *drawOpts, gfx_drawBuffer *buffer)
{
    gfx_drawTriangleOpts(&q->tris[0], mvp, drawOpts, buffer);
    gfx_drawTriangleOpts(&q->tris[1], mvp, drawOpts, buffer);
}

void drawScene(const Scene *s, const mth_Matrix4 *mvp, const gfx_drawOptions *drawOpts, gfx_drawBuffer *buffer)
{
    int w;
    static int bgOffset = 0;
    // background: Doom 1 sky texture, scrolling horizontally
    gfx_drawBitmapOffset(&s->textures[0], 0, 0, bgOffset % s->textures[0].width, 0, buffer);
    bgOffset++;

    // walls and sprites
    for(w = 0; w < NUM_WALLS; w++)
        drawSceneQuad(&s->walls[w], mvp, drawOpts, buffer);

    // push to screen!
    gfx_updateScreen(buffer);
    gfx_vSync();
}

void setupScene(Scene *s)
{
    int i, k;
    gfx_Bitmap textureAtlas = gfx_loadBitmap("images/scene.bmp");
    gfx_Bitmap skyTexture   = gfx_bitmapFromAtlas(&textureAtlas, 0, 0, 256, 128);

    s->textures[0] = gfx_resizeBitmap(&skyTexture, 320, 128);
    s->textures[1] = gfx_bitmapFromAtlas(&textureAtlas, 0, 256, 128, 128);
    s->textures[2] = gfx_bitmapFromAtlas(&textureAtlas, 0, 128, 128, 128);
    s->textures[3] = gfx_bitmapFromAtlas(&textureAtlas, 128, 128, 128, 128);
    s->textures[4] = gfx_bitmapFromAtlas(&textureAtlas, 128, 256, 128, 128);
    s->textures[5] = gfx_bitmapFromAtlas(&textureAtlas, 0, 384, 128, 128);
    s->textures[6] = gfx_bitmapFromAtlas(&textureAtlas, 128, 384, 128, 128);
    
    // atlas bitmap is no longer needed at this point
    gfx_freeBitmap(&textureAtlas);

    // floor
    setupSceneQuad(&s->walls[0], -40, -20, 0, 40, 20, 0, 2.f, 2.f, &s->textures[2]);

    //rotate the floor
    for(k = 0; k < 2; ++k)
    {
        for(i = 0; i < 3; i++)
        {
            mth_rotateVecAxisAngle(&s->walls[0].tris[k].vertices[i].position, 90.f * M_PI / 180.f, 1.f, 0.f, 0.f);
        }
    } 

    // back wall
    setupSceneQuad(&s->walls[1], -80, -120, -120, 80, 40, -120, 1.f, 1.f, &s->textures[1]);
    // right wall
    setupSceneQuad(&s->walls[2],  80, -40, -120, 100, 40, -40, 1.f, 1.f, &s->textures[3]);
    // left wall
    setupSceneQuad(&s->walls[3], -100, -40, -40, -80, 40, -120, 1.f, 1.f, &s->textures[3]);
    // baron of hell
    setupSceneQuad(&s->walls[4], 20, -20, -65, 84, 44, -65, 1.f, 1.f, &s->textures[4]);
    // cacodemon
    setupSceneQuad(&s->walls[5], -60, -20, -95, 4, 44, -95, 1.f, 1.f, &s->textures[5]);
    // shotgun guy
    setupSceneQuad(&s->walls[6], -100, -20, -42, -34, 44, -42, 1.f, 1.f, &s->textures[6]);
}

void freeScene(Scene *s)
{
    int w;
    for(w = 0; w < NUM_TEXTURES; w++)
        gfx_freeBitmap(&s->textures[w]);
}
