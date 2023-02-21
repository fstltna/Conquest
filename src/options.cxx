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
#include "global.h"
#include "context.h"
#include "conqdef.h"
#include "cb.h"
#include "color.h"
#include "ui.h"
#include "conf.h"
#include "clntauth.h"
#include "cd2lb.h"
#include "iolb.h"
#include "client.h"
#include "clientlb.h"
#include "conqutil.h"
#include "cprintf.h"


static void ChangeOption(struct Conf *cdata, int lin);
static int ViewEditOptions(struct Conf ConfigData[], int ConfSize,
			   int editable);
static void DisplayHelpScreen(struct Conf *confitem);

static int ChangedSomething = false;


/*************************************************************************
 * SysOptsMenu() - display System options menu
 *
 *
 *************************************************************************/

void SysOptsMenu(void)
{
    ChangedSomething = false;
    ViewEditOptions(SysConfData, SysCfEnd, true);
    if (ChangedSomething == true)
        SaveSysConfig();
    return;
}


/*************************************************************************
 * ChangeOption() - change an option (or prompt for new value)
 *
 *
 *************************************************************************/

static void ChangeOption(struct Conf *cdata, int lin)
{
#define CBUFLEN 1024
    int j, rv;
    char buf[CBUFLEN];

    switch(cdata->ConfType)
    {
    case CTYPE_BOOL:
        j = *(int *)(cdata->ConfValue);

        if (j == true)
            j = false;
        else
            j = true;

        *(int *)(cdata->ConfValue) = j;

        /* signal that something has been changed */
        ChangedSomething = true;

        break;

        /* these will need prompting on line 'lin' */
    case CTYPE_STRING:
        if (cdata->max > CBUFLEN)
	{
            utLog("ChangeOption: conf data max exceeds local buffer size.");
            break;
	}
        cdclrl(lin, 1);
        utStrncpy(buf, ((char *)cdata->ConfValue), CBUFLEN);
        rv = cdgets("Value: ", lin, 1, buf, cdata->max - 1);

        if (rv != -1)
	{
            utStrncpy((char *)cdata->ConfValue, buf, cdata->max);
            /* signal that something has been changed */
            ChangedSomething = true;
	}

        break;

    case CTYPE_NUMERIC:
        cdclrl(lin, 1);
        rv = cdgetn("Enter a number: ", lin, 1, &j);

        if (rv != -1)
	{
            if (j >= cdata->min && j <= cdata->max)
	    {
                *(int *)(cdata->ConfValue) = j;
                /* signal that something has been changed */
                ChangedSomething = true;
	    }
	}

        break;

    case CTYPE_UNUMERIC:
        cdclrl(lin, 1);
        rv = cdgetn("Enter a number: ", lin, 1, &j);

        if (rv != -1)
	{
            if (j >= cdata->min && j <= cdata->max)
	    {
                *(uint_t *)(cdata->ConfValue) = uint_t(j);
                /* signal that something has been changed */
                ChangedSomething = true;
	    }
	}

        break;
    }

    return;
}


/*************************************************************************
 * ViewEditOptions() - View or edit an options array
 *
 *
 *************************************************************************/

static int ViewEditOptions(struct Conf ConfigData[], int ConfSize,
			   int editable)
{
    int i, j, k;
    static const char *uheader = "User Configurable Options";
    static const char *sheader = "System-wide Configurable Options";
    static const char *header;
    static const char *eprompt = "Arrow keys to select an item, [SPACE] to change, any other key to quit.";
    static const char *vprompt = MTXT_DONE;
    static const char *eprompt2 = "Type '?' for help on an item.";
    int ch;
    int lin = 0, col = 0, flin, llin = 0, clin;
    const int settingcol = 65;
    int vattrib;
#define MAXOPTCOLS 128
    int cvec[MAXOPTCOLS];		/* hopefully big enough */
    int usingSysOpts;

    usingSysOpts = ((ConfigData == SysConfData) ? true : false);

    if (usingSysOpts)
        header = sheader;
    else
        header = uheader;

    cdclear();			/* First clear the display. */

    flin = 4;			/* first option line */

#ifdef DEBUG_OPTIONS
    int maxllin = MSG_LIN1 - 2;	/* max last option line */
#endif

    clin = 0;			/* current option line (also idx into cvec) */

				/* clear out cvec */

    for (i=0; i < MAXOPTCOLS; i++)
        cvec[i] = -1;

    bool done = false;
    while (!done)
    {
        cdclrl( 1, MSG_LIN2);	/* clear screen area */
        lin = 1;
        col = ((int)(Context.maxcol - strlen(header))/ 2);

        cprintf(lin, col, ALIGN_NONE, "#%d#%s", NoColor, header);

        lin = flin;
        col = 1;

        i = 0;			/* start at index 0 */
        k = 0;			/* init cvec */

        while (i < ConfSize)
	{
#ifdef DEBUG_OPTIONS
            utLog("ViewEditOptions(): ConfSize = %d, i = %d, type = %d, %s",
                  ConfSize, i, ConfigData[i].ConfType,
                  ConfigData[i].OneLineDesc);
#endif
            if (ConfigData[i].ConfType != CTYPE_STRING &&
                ConfigData[i].ConfType != CTYPE_BOOL &&
                ConfigData[i].ConfType != CTYPE_NUMERIC &&
                ConfigData[i].ConfType != CTYPE_UNUMERIC)
	    {		/* skip special elements */
#ifdef DEBUG_OPTIONS
                utLog("ViewEditOptions():\tSKIPPING");
#endif
                i++;
                continue;
	    }

            cvec[k++] = i;

            cprintf(lin, col, ALIGN_NONE, "#%d#%s#%d#",
                    InfoColor, ConfigData[i].OneLineDesc,
                    NoColor);

            switch(ConfigData[i].ConfType)
	    {
	    case CTYPE_BOOL:
                j = *(int *)ConfigData[i].ConfValue;

                if (j == true)
                    vattrib = GreenLevelColor;
                else
                    vattrib = RedLevelColor;

                cprintf(lin, settingcol, ALIGN_NONE, "#%d#%s#%d#",
                        vattrib, (j == true) ? "True" : "False",
                        NoColor);
                break;

	    case CTYPE_STRING:
                cprintf(lin, settingcol, ALIGN_NONE, "#%d#%13s#%d#",
                        InfoColor, (char *) ConfigData[i].ConfValue,
                        NoColor);
                break;

	    case CTYPE_NUMERIC:
                j = *(int *)ConfigData[i].ConfValue;
                cprintf(lin, settingcol, ALIGN_NONE, "#%d#%d#%d#",
                        NoColor, j,
                        NoColor);
                break;

	    case CTYPE_UNUMERIC:
            {
                uint_t u = *(int *)ConfigData[i].ConfValue;
                cprintf(lin, settingcol, ALIGN_NONE, "#%d#%u#%d#",
                        NoColor, u,
                        NoColor);
            }
            break;


	    } /* switch */
            lin++;
            i++;
	}

        /* now the editing phase */
        llin = k - 1; // last option line

#ifdef DEBUG_OPTIONS
        utLog("ViewEditOptions(): maxllin = %d, llin = %d, k = %d", maxllin, llin, k);
#endif

        cdclrl( MSG_LIN1, 2  );
        if (editable)
	{
            cdputs(eprompt, MSG_LIN1, 1);
            cdputs(eprompt2, MSG_LIN2, 1);
            cdmove(flin + clin, 1);
	}
        else
            cdputc(vprompt, MSG_LIN1);

        /* Get a char */
        ch = iogchar();

        if (!editable)
	{
            break;		/* exit here after viewing */
	}

        switch(ch)
	{
	case KEY_UP:		/* up */
	case KEY_LEFT:
	case 'w':
	case 'k':
            clin--;
            if (clin < 0)
	    {
                clin = llin;
	    }
            break;

	case KEY_DOWN:		/* down */
	case KEY_RIGHT:
	case 'x':
	case 'j':
            clin++;
            if (clin > llin)
	    {
                clin = 0;
	    }
            break;

	case ' ':	/* change something */
            ChangeOption(&ConfigData[cvec[clin]], MSG_LIN1);
            break;

	case '?':
            DisplayHelpScreen(&ConfigData[cvec[clin]]);
            break;

	default:		/* everything else */
            done = true;
            break;
	}

#ifdef DEBUG_OPTIONS
        utLog("ViewEditOptions():###\tllin = %d, clin = %d", llin, clin);
#endif
    }

    return true;
}

/* DisplayHelpScreen() - display a help (actually the conf item comment)
 *                       screen for a Configuration item.
 */
static void DisplayHelpScreen(struct Conf *confitem)
{
    int i, col, lin;

    cdclear();

    lin = 1;
    col = 0;

    cprintf(lin, col, ALIGN_CENTER, "#%d#%s#%d#",
            NoColor, confitem->OneLineDesc, NoColor);

    lin += 2;

    i = 0;
    uiPutColor(InfoColor);

    while (confitem->ConfComment[i] != NULL)
    {
        if (confitem->ConfComment[i][0] != 0)
	{
            cdputs(&(confitem->ConfComment[i][1]), lin, col);
            lin++;
	}
        i++;
    }

    uiPutColor(NoColor);
    cdclrl( MSG_LIN1, 2  );
    cdputc(MTXT_DONE, MSG_LIN2);

    /* Get a char */
    (void)iogchar();

    return;
}
