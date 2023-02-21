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

#include "gameDir.h"

#ifndef GLOBAL_H
#define GLOBAL_H

/* for the semaphores */
#define LOCKMSG      (0)	/* lock the message portion  */
#define LOCKCMN      (1)	/* lock everything else  */


#if defined(NOEXTERN_GLOBALS)
int        ConquestGID;         /* Conquest's GID */

int        cqDebug = 0;         // debug/verbose level

gameDir gameSubdirectory;       // global game sub directory (relative
                                // to <prefix>/ etc/ and var/
                                // conquest/)

#else

extern int ConquestGID;

extern int cqDebug;

extern gameDir gameSubdirectory;

#endif

/* Home dir/appdir env variable(s) */
#if defined(MINGW)
# define CQ_USERHOMEDIR  "APPDATA"
# define CQ_USERCONFDIR  "Conquest"
#else
# define CQ_USERHOMEDIR  "HOME"
# define CQ_USERCONFDIR  ".conquest"
#endif


#endif /* GLOBAL_H */
