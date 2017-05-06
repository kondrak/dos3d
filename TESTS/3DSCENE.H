#include "src/bitmap.h"
#include "src/camera.h"
#include "src/triangle.h"
#include "src/math.h"
#include <stdlib.h>

typedef struct SceneQuad SceneQuad;

typedef struct SceneQuad
{
    Bitmap *texture;
    Triangle tris[2];
    SceneQuad *screenQuad;
} SceneQuad;

#define COLOR_MAGENTA 212
#define NUM_WALLS 7
#define NUM_TEXTURES 7

typedef struct
{
    Bitmap textures[NUM_TEXTURES];
    SceneQuad walls[NUM_WALLS];
} Scene;

// helper functions
void setupSceneQuad(SceneQuad *q, int qx, int qy, int qz, int qx2, int qy2, int qz2, float u, float v);
void drawSceneQuad(SceneQuad *q, unsigned char *buffer);
void setupScene(Scene *s);
void updateScene(Scene *s, Matrix4f *mvp);
void drawScene(Scene *s, unsigned char *buffer);
void freeScene(Scene *s);

// Render a simple scene
void test3DScene()
{
    unsigned short *keysPressed;
    Scene scene;
    Camera cam;
    Matrix4f model;
    Matrix4f modelViewProj;
    unsigned char *buffer = (unsigned char *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT);

    setupScene(&scene);
    
    if(!buffer)
    {
        printf("Out of memory!");
        return;
    }

    matIdentity(&model);
    matIdentity(&modelViewProj);

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

    keysPressed = translateInput();

    while(!keysPressed[KEY_ESC])
    {
        matView(&cam.view, &cam.position, &cam.target, &cam.up);
        matPerspective(&cam.projection, 75.f * M_PI /180.f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 5.f);
        modelViewProj = matMul(&cam.view, &cam.projection);
        modelViewProj = matMul(&model, &modelViewProj);      

        // update position of scene items
        updateScene(&scene, &modelViewProj);

        // render!
        clrScrBuffer(buffer);
        drawScene(&scene, buffer);
        updateScreen(buffer);

        keysPressed = translateInput();
    }

    free(buffer);
    freeScene(&scene);

    do {
        keysPressed = translateInput();
    } while(keysPressed[KEY_ESC]);
}


void setupSceneQuad(SceneQuad *q, int qx, int qy, int qz, int qx2, int qy2, int qz2, float u, float v)
{
    q->tris[0].color = 1;
    q->tris[0].texture = q->texture;
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
    q->tris[1].texture = q->texture;
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
    
    q->screenQuad = (SceneQuad*)malloc(sizeof(SceneQuad));
    q->screenQuad->texture = q->texture;
    q->screenQuad->tris[0] = q->tris[0];
    q->screenQuad->tris[1] = q->tris[1]; 
}

void drawSceneQuad(SceneQuad *q, unsigned char *buffer)
{
    drawTriangleColorKey(&q->tris[0], buffer, COLOR_MAGENTA);
    drawTriangleColorKey(&q->tris[1], buffer, COLOR_MAGENTA);
}

void updateScene(Scene *s, Matrix4f *mvp)
{
    int i, k, w;
    for(w = 0; w < NUM_WALLS; w++)
    {
        for(k = 0; k < 2; k++)
        {
            for(i = 0; i < 3; i++)
            {
                s->walls[w].screenQuad->tris[k].vertices[i].position = matMulVec(mvp, &s->walls[w].tris[k].vertices[i].position);
                s->walls[w].screenQuad->tris[k].vertices[i].position.x = (s->walls[w].screenQuad->tris[k].vertices[i].position.x * (float)SCREEN_WIDTH)  / (2.0f * s->walls[w].screenQuad->tris[k].vertices[i].position.w) + (SCREEN_WIDTH >> 1);
                s->walls[w].screenQuad->tris[k].vertices[i].position.y = (s->walls[w].screenQuad->tris[k].vertices[i].position.y * (float)SCREEN_HEIGHT) / (2.0f * s->walls[w].screenQuad->tris[k].vertices[i].position.w) + (SCREEN_HEIGHT >> 1);
            }
        }
    }
}

void drawScene(Scene *s, unsigned char *buffer)
{
    int w;
    // background: Doom 1 sky texture
    drawBitmap(&s->textures[0], 0, 0, buffer);

    // walls and sprites
    for(w = 0; w < NUM_WALLS; w++)
        drawSceneQuad(s->walls[w].screenQuad, buffer);

    // push to screen!
    updateScreen(buffer);
    vsync();
}

void setupScene(Scene *s)
{
    int i, k;
    Bitmap textureAtlas = loadBitmap("images/scene.bmp");
    Bitmap skyTexture = bitmapFromAtlas(&textureAtlas, 0, 0, 256, 128);
    s->textures[0] = resizeBitmap(&skyTexture, 320, 128);
    s->textures[1] = bitmapFromAtlas(&textureAtlas, 0, 256, 128, 128);
    s->textures[2] = bitmapFromAtlas(&textureAtlas, 0, 128, 128, 128);
    s->textures[3] = bitmapFromAtlas(&textureAtlas, 128, 128, 128, 128);
    s->textures[4] = bitmapFromAtlas(&textureAtlas, 128, 256, 128, 128);
    s->textures[5] = bitmapFromAtlas(&textureAtlas, 0, 384, 128, 128);
    s->textures[6] = bitmapFromAtlas(&textureAtlas, 128, 384, 128, 128);
    freeBitmap(&textureAtlas);

    s->walls[0].texture = &s->textures[2];
    s->walls[1].texture = &s->textures[1];
    s->walls[2].texture = &s->textures[3];
    s->walls[3].texture = &s->textures[3];
    s->walls[4].texture = &s->textures[4];
    s->walls[5].texture = &s->textures[5];
    s->walls[6].texture = &s->textures[6];
    // floor
    setupSceneQuad(&s->walls[0], -40, -20, 0, 40, 20, 0, 2.f, 2.f);

    //rotate the floor
    for(k = 0; k < 2; ++k)
    {
        for(i = 0; i < 3; i++)
        {
            rotateVecAxisAngle(&s->walls[0].tris[k].vertices[i].position, 90.f * M_PI / 180.f, 1.f, 0.f, 0.f);
        }
    } 

    // back wall
    setupSceneQuad(&s->walls[1], -80, -120, -120, 80, 40, -120, 1.f, 1.f);
    // right wall
    setupSceneQuad(&s->walls[2],  80, -40, -120, 100, 40, -40, 1.f, 1.f);
    // left wall
    setupSceneQuad(&s->walls[3], -100, -40, -40, -80, 40, -120, 1.f, 1.f);
    // baron of hell
    setupSceneQuad(&s->walls[4], 20, -20, -65, 84, 44, -65, 1.f, 1.f);
    // cacodemon
    setupSceneQuad(&s->walls[5], -60, -20, -95, 4, 44, -95, 1.f, 1.f);
    // shotgun guy
    setupSceneQuad(&s->walls[6], -100, -20, -42, -34, 44, -42, 1.f, 1.f);
}

void freeScene(Scene *s)
{
    int w;
    for(w = 0; w < NUM_TEXTURES; w++)
        freeBitmap(&s->textures[w]);
    
    for(w = 0; w < NUM_WALLS; w++)
        free(s->walls[w].screenQuad);
}
