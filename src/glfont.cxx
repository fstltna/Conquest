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


#include "c_defs.h"
#include "conqdef.h"
#include "context.h"
#include "global.h"
#include "color.h"
#include "ui.h"
#include "cb.h"
#include "ibuf.h"
#include "gldisplay.h"
#include "conf.h"
#include "conqutil.h"

#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "glmisc.h"

#define NOEXTERN_GLFONT
#include "glfont.h"
#undef NOEXTERN_GLFONT

#include <assert.h>

void glfInitFonts(void)
{
    char fbuf[PATH_MAX] = {};
    int fail = false;

#ifdef DEBUG_GL
    utLog("%s: ENTER...", __FUNCTION__);
#endif

    snprintf(fbuf, PATH_MAX, "%s/img/%s", utGetPath(CONQSHARE), "large.txf");

    glfFontLarge = txfLoadFont(fbuf);

    if (!glfFontLarge)
    {
        utLog("glfInitFonts: load %s failed", fbuf);
        fprintf(stderr, "glfInitFonts: load %s failed\n", fbuf);
        fail = true;
    }

    sprintf(fbuf, "%s/img/%s", utGetPath(CONQSHARE), "fixed.txf");
    glfFontFixed = txfLoadFont(fbuf);

    if (!glfFontFixed)
    {
        utLog("glfInitFonts: load %s failed", fbuf);
        fprintf(stderr, "glfInitFonts: load %s failed\n", fbuf);
        fail = true;
    }

    sprintf(fbuf, "%s/img/%s", utGetPath(CONQSHARE), "tinyfixed.txf");
    glfFontFixedTiny = txfLoadFont(fbuf);

    if (!glfFontFixedTiny)
    {
        utLog("glfInitFonts: load %s failed", fbuf);
        fprintf(stderr, "glfInitFonts: load %s failed\n", fbuf);
        fail = true;
    }

    sprintf(fbuf, "%s/img/%s", utGetPath(CONQSHARE), "msg.txf");
    glfFontMsg = txfLoadFont(fbuf);

    if (!glfFontMsg)
    {
        utLog("glfInitFonts: load %s failed", fbuf);
        fprintf(stderr, "glfInitFonts: load %s failed\n", fbuf);
        fail = true;
    }

    if (fail)
        exit(1);

    txfEstablishTexture(glfFontLarge, 0, GL_TRUE);
    GLError();
    txfEstablishTexture(glfFontFixed, 0, GL_TRUE);
    GLError();
    txfEstablishTexture(glfFontFixedTiny, 0, GL_TRUE);
    GLError();
    txfEstablishTexture(glfFontMsg, 0, GL_TRUE);
    GLError();
    return;
}

void glfRenderFont(GLfloat x, GLfloat y, GLfloat z, GLfloat w, GLfloat h,
                   TexFont *font, const char *str, int color, GLColor_t *glcol,
                   uint32_t flags)
{
    GLfloat inverty = ((flags & GLF_FONT_F_ORTHO) ? -1.0 : 1.0);
    int width, ascent, descent;
    GLfloat xs = 1.0, ys = 1.0;

    int l;

    if (!font)
        return;

    if (!str)
        return;

    l = strlen(str);

    txfBindFontTexture(font);
    GLError();
    txfGetStringMetrics( font, str, l, &width,
                         &ascent, &descent);
    if (flags & GLF_FONT_F_SCALEX)
        xs = w/(GLfloat)width;

    ys = ((2.0 * (h/(ascent + descent))) * 0.70);


#if 0
    utLog("glfRenderFont(%s): WINDOW: %d, w = %f h = %f, \n"
          "\twid = %d, asc = %d desc = %d"
          "\ttxf height = %d",
          str,
          glutGetWindow(), w, h, width, ascent, descent,
          font->tex_height);
#endif

    /* remove the glyph backgrounds */
    glAlphaFunc(GL_GREATER, 0.2);
    glEnable(GL_ALPHA_TEST);

    glEnable(GL_TEXTURE_2D);

    glPushMatrix();

    if (flags & GLF_FONT_F_ORTHO)
    {
        glTranslatef( x, y + h, z );
        glScalef(xs, inverty * ys, 1.0);
    }
    else
    {
        glTranslatef( x, y - h, z );
        glScalef(xs, inverty * ys, 1.0);
    }

    if (glcol)
    {
        glColor4f(glcol->r,
                  glcol->g,
                  glcol->b,
                  glcol->a);
    }
    else
        uiPutColor(color);

    if (flags & GLF_FONT_F_DOCOLOR)
        txfRenderFancyString(font, str, l);
    else
        txfRenderString(font, str, l);

    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_ALPHA_TEST);

    return;
}
