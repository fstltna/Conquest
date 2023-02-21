//
// Author: Jon Trulson <jon@radscan.com>
// Copyright (c) 1994-2018 Jon Trulson
//
// The MIT License
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
// BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifndef _C_GL_H
#define _C_GL_H

#include "textures.h"
#include "anim.h"

/* storage for ship textures, colors, other GL related things, indexed by
   team/shiptype */
typedef struct _glship {
    textureIdx_t ship;             /* main ship texture */
    textureIdx_t sh;               /* main ship shield texture */
    textureIdx_t phas;             /* phaser */
    textureIdx_t ico;              /* ship icon */
    textureIdx_t ico_sh;           /* icon shields id */
    textureIdx_t ico_torp;         /* torp pip */
    textureIdx_t decal1;           /* sh/hull gauges */
    textureIdx_t decal1_lamp_sh;   /* sh lamp */
    textureIdx_t decal1_lamp_hull; /* hull lamp */
    textureIdx_t decal1_lamp_fuel; /* fuel lamp */
    textureIdx_t decal1_lamp_eng;  /* eng lamp */
    textureIdx_t decal1_lamp_wep;  /* wep lamp */
    textureIdx_t decal1_lamp_rep;  /* repairing lamp */
    textureIdx_t decal1_lamp_cloak;/* cloaking lamp */
    textureIdx_t decal1_lamp_tow;  /* towing/towedby lamp */
    textureIdx_t decal2;           /* fuel/w/e/alloc/kills gauges */
    textureIdx_t dial;             /* ships astrogator dial */
    textureIdx_t dialp;            /* astrogator pointer */
    textureIdx_t warp;             /* warp gauge */
    textureIdx_t warp2;            /* warp gauge background*/
    textureIdx_t warpq_col;        /* warp indicator quad (color only) */
} GLShip_t;

#if defined(NOEXTERN_GL)
GLShip_t GLShips[NUM_PLAYERTEAMS][MAXNUMSHIPTYPES] = {};

/* the LR scaling array (-5 - 5) */
const GLfloat scaleFactorsLR[10] = {
    /* zoom out (mag-) */
    0.75,
    0.8,
    0.85,
    0.9,
    0.95,
    /* zoom in (mag+) */
    1.25,
    1.3,
    1.35,
    1.4,
    1.45
};

/* the SR scaling array (-5 - 5) */
const GLfloat scaleFactorsSR[10] = {
    /* zoom out (mag-) */
    0.5,
    0.6,
    0.7,
    0.8,
    0.9,
    /* zoom in (mag+) */
    1.45,
    1.90,
    2.35,
    2.80,
    3.25
};

// The defaultTexture index - always created first, always 0
const textureIdx_t defaultTextureIdx = 0;

/* count resize/scale change events so functions that care, can detect
 * them and do the right thing.
 */
uint32_t GLGeoChange = 0;

// tactical ring colors
textureIdx_t tacRing1K, tacRing2K, tacRing3K, tacRingXK, tacRing10K;

#else
extern GLShip_t GLShips[NUM_PLAYERTEAMS][MAXNUMSHIPTYPES];

extern GLfloat  scaleFactorsLR[10];
extern GLfloat  scaleFactorsSR[10];
extern const textureIdx_t defaultTextureIdx;
extern const uint32_t GLGeoChange;
extern textureIdx_t tacRing1K, tacRing2K, tacRing3K, tacRingXK, tacRing10K;
#endif

void drawPlanet( GLfloat x, GLfloat y, int pnum, int scale,
                 int textcolor );
real cu2GLSize(real size, int scale);

int GLcvtcoords(real cenx, real ceny, real x, real y, real scale,
                GLfloat *rx, GLfloat *ry );

void drawTorp(GLfloat x, GLfloat y, int scale,
              int snum, int torpnum);
void drawShip(GLfloat x, GLfloat y, GLfloat angle, char ch, int i,
	      int color, GLfloat scale);
void drawDoomsday(GLfloat x, GLfloat y, GLfloat angle, GLfloat scale);
void drawViewerBG(int snum, int dovbg);
void drawNEB(int snum);
float getFPS(void);

void drawLine(GLfloat x, GLfloat y, GLfloat len, GLfloat lw);
void drawCircle(float x, float y, float r, int num_segments);
void drawLineBox(GLfloat x, GLfloat y, GLfloat z,
                 GLfloat w, GLfloat h, int color,
                 GLfloat lw);
void drawQuad(GLfloat x, GLfloat y, GLfloat w, GLfloat h, GLfloat z);
void drawTexQuad(GLfloat x, GLfloat y, GLfloat z, GLfloat w, GLfloat h,
                 int ortho, int rot90);
void drawTexBoxCentered(GLfloat x, GLfloat y, GLfloat z, GLfloat size,
                        int ortho, int rot90);
void drawExplosion(GLfloat x, GLfloat y, int snum, int torpnum, int scale);
void drawBombing(int snum, int scale);

void dspInitData(void);

void drawIconHUDDecal(GLfloat rx, GLfloat ry, GLfloat w, GLfloat h,
                      textureHUDItem_t imgp, cqColor icol);
void hex2GLColor(uint32_t hcol, GLColor_t *col);
void setViewerScaling(int scale, int isLR);

#endif /* _C_GL_H */
