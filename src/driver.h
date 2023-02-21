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


#ifndef DRIVER_H_INCLUDED
#define DRIVER_H_INCLUDED

#include "conqdef.h"

/* Driver states. */
enum DriverState : int {
    DRS_OFF = 0,               /* normal off mode */
    DRS_RESTART = 1,           /* required state for starting a driver */
    DRS_STARTING = 2,          /* transitory starting state */
    DRS_RUNNING = 3,           /* normal running mode */
    DRS_HOLDING = 4,           /* don't execute by will of GOD */
    DRS_KAMIKAZE = 5,          /* used to kill a driver */
};

typedef struct {
    DriverState  drivstat; /* driver status; one of the DRS_ values */
    int  drivpid;  /* pid of the driver process */
    int  drivcnt;  /* number of driver timeouts */
    int  drivsecs; /* phase of the universe in seconds (mod 5 mins) */
    char drivowner[MAX_USERNAME]; /* username of the driver's owner */

    // The following are seconds since the epoch.
    time_t drivtime;     /* last time the driver looped */
    time_t playtime;     /* last time a player looped */
} Driver_t;

#endif /* DRIVER_H_INCLUDED */
