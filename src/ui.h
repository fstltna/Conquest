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

#ifndef _UI_H
#define _UI_H

#include <string>

// ui specific - these will be found in files placed into the UiGL
// (OpenGL) or UiCU (curses) specific library files

// color{CU,GL}
void uiInitColors(void);
void uiPutColor(cqColor col);
// planet updating (textures, etc) for the GL client, this is defined
// in GL.c, for the curses client this will be a noop defined in
// cumisc.c
int uiUpdatePlanet(int pnum);
// uiCU.cxx or uiGL.cxx
void uiMoveCursor(int lin, int col);
// print a message at the given line
void uiPutMsg(const std::string& buf, int line);

// ui general, these are found in ui*.cxx files, meant to be generic
// WRT drawing technology (curse/OGL) in use

// returns the length of a string, not counting embedded color codes
// ("#%d#")
int uiCStrlen(const std::string& buf);

#endif /* _UI_H */
