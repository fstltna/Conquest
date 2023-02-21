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
#define NOEXTERN_CB
#include "cb.h"                 /* common block vars defined here */

#include "global.h"
#include "conqdef.h"
#include "conqlb.h"
#include "conqutil.h"
#include "conqunix.h"

#if !defined(MINGW)
# include <sys/mman.h>
#endif

#include "context.h"		/* some extra stuff */
#include "user.h"
#include "sem.h"

static char *_cbBasePtr = NULL;	/* common block base ptr, the Universe. */
static unsigned int _cbOffset = 0; /* offset into common block */
static unsigned int _cbSavedSize = 0; /* for msync() and friends */

static bool fakeCommon = false;	/* for the clients */

/* Some architectures do not like unaligned accesses (like sparc) so
 * we need to ensure proper alignment of the structures contained
 * within the common block.  We will use 16-byte alignment, which
 * should work for everybody.
 */

#define CB_ALIGNMENT            (16)
#define CB_ALIGN(_off, _align)  ( ((_off) + (_align)) & ~((_align) - 1) )

// This macro maps a global variable into the memory region starting
// at _cbBasePtr.  The memory region is either a heap-allocated chunk
// of memory (in the case of the clients), or a memory mapped file
// (via mmap()) used for persistent storage (on the server only).  This
// emulation of a Fortran "Common Block" is used to hold the state of
// the Universe.  Be careful with it.

#define MAP_VARIABLE(thevarp, thetype, size, doAssign) {                \
        if (doAssign) { thevarp = (thetype *) (_cbBasePtr + _cbOffset); } \
        _cbOffset += (sizeof(thetype) * (size));                          \
        _cbOffset = CB_ALIGN(_cbOffset, CB_ALIGNMENT);                    \
    }

// Static functions...

static int _checkCB(char *fname, int fmode, int sizeofcb);

/* my malloc wrapper. used only when mapping or initializing a
   commonblock */
static void *_mymalloc(size_t size)
{
    void *ptr;

    if ((ptr = malloc(size)) == NULL)
    {
	perror("_mymalloc()");
	exit(1);
    }
    return(ptr);
}

/* maps the actual vars into the common block, if doAssign is set.
 * Otherwise, this can be used to determine how many bytes are
 * required for the CB by only modifying the offset during mapping */
static void _mapCBVariables(bool doAssign)
{
    _cbOffset = 0;

    // This must be the first var
    MAP_VARIABLE(cbRevision, unsigned int, 1, doAssign);

    MAP_VARIABLE(cbConqInfo, cbConqInfo_t, 1, doAssign);

    MAP_VARIABLE(cbUsers, User_t, cbLimits.maxUsers(), doAssign);

    MAP_VARIABLE(cbRobot, Robot_t, 1, doAssign);

    MAP_VARIABLE(cbPlanets, Planet_t, cbLimits.maxPlanets(), doAssign);

    MAP_VARIABLE(cbTeams, Team_t, NUM_ALLTEAMS, doAssign);

    MAP_VARIABLE(cbDoomsday, Doomsday_t, 1, doAssign);

    MAP_VARIABLE(cbHistory, History_t, cbLimits.maxHist(), doAssign);

    MAP_VARIABLE(cbDriver, Driver_t, 1, doAssign);

    MAP_VARIABLE(cbShips, Ship_t, cbLimits.maxShips(), doAssign);

    MAP_VARIABLE(cbShipTypes, ShipType_t, MAXNUMSHIPTYPES, doAssign);

    MAP_VARIABLE(cbMsgs, Msg_t, cbLimits.maxMsgs(), doAssign);

    // if we did actual assignments, save the offset
    if (doAssign)
        _cbSavedSize = _cbOffset;

    return;
}

static void _unmapCBVariables()
{
    // this simply sets all the CB variables to NULL.  It should only
    // be called by cbUnmap() and cbUnmapLocal()

    // This must be the first var
    cbRevision = NULL;

    cbConqInfo = NULL;

    cbUsers = NULL;

    cbRobot = NULL;

    cbPlanets = NULL;

    cbTeams = NULL;

    cbDoomsday = NULL;

    cbHistory = NULL;

    cbDriver = NULL;

    cbShips = NULL;

    cbShipTypes = NULL;

    cbMsgs = NULL;

    return;
}

static void _initFakeCB(void)
{
    fakeCommon = true;

    /* this will exit if it fails */
    if (!_cbBasePtr)
        _cbBasePtr = (char *)_mymalloc(cbGetSize());

    _mapCBVariables(true);

    cbZero();
    return;
}

/* we'll use a hack to translate the lock[mesg|word] pointers into
   a semaphore selector */

void cbLock(int *lockptr)
{
    int semnum;

    if (lockptr == &cbConqInfo->lockmesg)
        semnum = LOCKMSG;
    else
        semnum = LOCKCMN;

    semLock(semnum);

    (*lockptr)++;

    return;
}

void cbUnlock(int *lockptr)
{
    int semnum;

    if (lockptr == &cbConqInfo->lockmesg)
        semnum = LOCKMSG;
    else
        semnum = LOCKCMN;

    semUnlock(semnum);

    return;
}

/* cbFlush() - flush a common block */
void cbFlush(void)
{
    if (fakeCommon)
        return;

#if !defined(MINGW)
    /* fbsd doesn't like MS_SYNC       */
    /* which is prefered, but oh well */
# if defined(FREEBSD)
    if (msync((caddr_t)_cbBasePtr, _cbSavedSize, 0) == -1)
# else
        if (msync((caddr_t)_cbBasePtr, _cbSavedSize, MS_SYNC) == -1)
# endif
            utLog("cbFlush(): msync(): %s", strerror(errno));
#endif

    return;
}

/* _checkCB() - open/verify a common block - init if necc, return true
   if successful */
static int _checkCB(char *fname, int fmode, int sizeofcb)
{
    int ffd = -1;
    struct stat sbuf;

    /* first stat the file, if it exists then verify the size.  rename
       it if a size mismatch */
    if (stat(fname, &sbuf) != -1)   /* ok if this fails */
    {
        // file exists - verify size
        if (sbuf.st_size != sizeofcb)
	{
            // we have a problem - we cannot use this CB.  So, try to
            // rename it to something else so we can create a fresh
            // new one.  By renaming the old one, we don't risk
            // throwing away an important CB due to someone fiddling
            // with the limits in cbGlobal.

            // we create a new file tagged with the unix time
            char newfile[PATH_MAX] = {};
            snprintf(newfile, PATH_MAX, "%s/%s-%lu",
                     CONQSTATE, C_CONQ_COMMONBLK,
                     time(0));

            utLog("%s: %s: ERROR: File size mismatch (expected %d, was %ld), "
                  "renaming to %s.",
                  __FUNCTION__,
                  fname,
                  sizeofcb,
                  (long int)sbuf.st_size,
                  newfile);

            if (rename(fname, newfile) == -1)
	    {
                utLog("%s: rename(%s, %s) failed: %s\n",
                      __FUNCTION__,
                      fname,
                      newfile,
                      strerror(errno));
                return(false);
	    }
	}
    }


    /* ok, either the file exists with the right
       size, or it doesn't exist at all -
       now open (and create) if necc */

    umask(0);			/* clear umask, just in case... */

    if ((ffd = open(fname, O_RDONLY)) == -1)
    {				/* Error or not there...  */
        if (errno == ENOENT)	/* Not There */
	{			/* create it */
            if ((ffd = creat(fname, fmode)) == -1)
	    {
                utLog("_checkCB(): creat(%s) failed: %s\n",
                      fname,
                      strerror(errno));
                return(false);
	    }
            else
	    {			/* Create it */
                utLog("%s: Initializing new common block: %s\n",
                      __FUNCTION__, fname);

                // this exits if malloc fails
                void *memptr = _mymalloc(sizeofcb);
                memset(memptr, 0, sizeofcb);

                if (write(ffd, memptr, sizeofcb) <= 0)
                {
                    utLog("_checkCB(): write() failed: %s\n",
                          strerror(errno));

                    close(ffd);
                    ffd = -1;
                    free(memptr);
                    memptr = NULL;
                    return false;
                }

                close(ffd);
                ffd = -1;
                free(_cbBasePtr);
                _cbBasePtr = NULL;
                free(memptr);
	    }
        }
        else
	{			/* some other error */
            utLog("_checkCB(): open(%s, O_RDONLY) failed: %s\n",
                  fname,
                  strerror(errno));
            return(false);
	}
    }

    if (ffd >= 0)
        close(ffd);			/* everything ok.. */

#if !defined(MINGW)
    /* set ownership */
    if (chown(fname, 0, -1) == -1)
    {
        // don't whine on EPERM.  Many systems don't allow
        // ordinary users to chown anymore
        if (errno != EPERM)
            utLog("_checkCB(): chown() failed: %s\n",
                  strerror(errno));
    }
#endif

    return(true);			/* everything there, and right size */
}

void cbMap(void)
{
#if !defined(MINGW)
    int cmn_fd;
    static char cmnfile[PATH_MAX] = {};
#endif

    if (fakeCommon)
        return;

    utLog("%s: Mapping the common block", __FUNCTION__);
#if defined(MINGW)
    fprintf(stderr,
            "%s: Only fake (client) common blocks are supported under MINGW\n",
        __FUNCTION__);
    exit(1);
#else  /* MINGW */

    utLog("%s: CB size: %u bytes", __FUNCTION__,
          cbGetSize());

    snprintf(cmnfile, PATH_MAX, "%s/%s/%s", CONQSTATE,
             gameSubdirectory.get().c_str(),
             C_CONQ_COMMONBLK);

    /* verify it's validity */
    if (_checkCB(cmnfile, CMN_MODE, cbGetSize()) == false)
        exit(1);			/* an unrecoverable error */

    /* reopen it... */
    if ((cmn_fd = open(cmnfile, O_RDWR)) == -1)
    {
        utLog("cbMap(): open(O_RDWR) failed: %s", strerror(errno));
        exit(1);
    }

    /* Now lets map it */

# ifndef MAP_FILE
#  define MAP_FILE 0		/* some arch's don't def this */
# endif

    if ((_cbBasePtr = (char *)mmap(NULL, (size_t) cbGetSize(),
                                   PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE,
                                   cmn_fd, 0)) == MAP_FAILED)
    {
        close(cmn_fd);
        utLog("cbMap(): mmap() failed: %s", strerror(errno));
        exit(1);
    }
#endif  /* MINGW */

    /* now map the variables into the
       common block */
    _mapCBVariables(true);

    // go ahead and close the fd, we don't need it anymore.
    close(cmn_fd);

    return;
}

void cbZero(void)
{				/* zero the common block, called from
				   init everything */
    memset(_cbBasePtr, 0, cbGetSize());
    upchuck();			/* flush the commonblock */

    return;
}

/* short cut */
void cbMapLocal(void)
{
    /* a parallel universe, it is */
    utLog("%s: CB size needed: %u bytes", __FUNCTION__,
          cbGetSize());
    utLog("%s: Mapping the common block", __FUNCTION__);

    _initFakeCB();
    clbInitEverything(true);
    clbInitMsgs();
    *cbRevision = COMMONSTAMP;
    cbDriver->drivstat = DRS_OFF;
    cbDriver->drivpid = 0;
    cbDriver->drivowner[0] = 0;

    return;
}

unsigned int cbGetSize()
{
    // Do a pretend map of the variables (at their current sizes) and
    // return the offset (_cbOffset).  Note, this number includes
    // any trailing alignment up to 16 bytes.

    _mapCBVariables(false);
    return (_cbOffset);
}

void cbUnmap()
{
    if (!_cbBasePtr)
        return;
    if (fakeCommon) // only for real file-backed mmapped common blocks
        return;

    utLog("%s: Unmapping the common block", __FUNCTION__);

    // reset all variables and unmap the common block

    _unmapCBVariables();
    // not much we can do if this fails...
    munmap((void *)_cbBasePtr, _cbSavedSize);
    _cbBasePtr = NULL;
    _cbOffset = 0;
    _cbSavedSize = 0;
}

void cbUnmapLocal()
{
    if (!_cbBasePtr)
        return;
    if (!fakeCommon) // only for heap allocated "fake" common blocks
        return;

    utLog("%s: Freeing the common block", __FUNCTION__);

    // reset all variables, free the "fake" common block

    _unmapCBVariables();
    // not much we can do if this fails...
    free((void *)_cbBasePtr);
    _cbBasePtr = NULL;
    _cbOffset = 0;
    _cbSavedSize = 0;
}

bool cbIsMapped()
{
    return ( (_cbBasePtr) ? true : false );
}
