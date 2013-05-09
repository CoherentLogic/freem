/***
 * $Source: /cvsroot-fuse/gump/FreeM/src/service.c,v $
 * $Revision: 1.8 $ $Date: 2000/02/28 18:02:23 $
 */
/*                            *
 *                           * *
 *                          *   *
 *                     ***************
 *                      * *       * *
 *                       *  MUMPS  *
 *                      * *       * *
 *                     ***************
 *                          *   *
 *                           * *
 *                            *
 *
 * Shalom ha-Ashkenaz, 1998/06/18 CE
 * 
 * mumps: various I/O stuff
 * 
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/timeb.h>

#define MAXZAS NESTLEVLS
#include "mpsdef.h"

#ifdef USE_SYS_TIME_H
#include <sys/time.h>
#endif

long int tell ();
unsigned alarm ();
unsigned int sleep ();
char   *calloc ();
void    ris ();

#ifndef FREEBSD
long int lseek ();
#endif

/* system services */

#ifdef SYSFIVE
#ifdef OLDUNIX
#include <termio.h>
#else
#include <termios.h>
#endif /* OLDUNIX */
#include <fcntl.h>
#else
#include <sgtty.h>
#endif /* SYSFIVE */

/******************************************************************************/

long int
find (string, pattrn)
/* search 'string' for occurence of 'pattrn'
 * return: 0='not found' nbr='pattern begins at nbr' */

	char   *string,
	       *pattrn;

{
    short   i,
            j;
    register k;
    register l;

    i = 0;
    while (pattrn[i] != EOL)
	i++;				/* $l of 2nd arg */

    if (i == 1) {
	l = pattrn[0];
	k = 0;
	while (string[k] != EOL)
	    if (string[k++] == l)
		return k;
	return 0L;
    }
    j = stlen (string);
    for (k = 0; k < j; k++) {
	l = 0;
      l10:if (string[k + l] != pattrn[l])
	    continue;
	if (++l < i)
	    goto l10;
	return ++k;
    }
    return 0L;
}					/* end of find() */

/******************************************************************************/
short int
kill_ok (exceptions, variable)
/* called by exclusive KILL to search 'exceptions' for occurence of 'variable'
 * return: 1='not found, can be killed' 0='cannot be killed' */
	char   *exceptions,
	       *variable;
{
    short   i,
            j;
    register k;
    register l;

    j = stlen (exceptions);
    i = stlen (variable);
    for (k = 0; k < j; k++) {
	for (l = 0; l < i; l++) {
	    if (exceptions[k + l] != variable[l]) {
		if (exceptions[k + l] == SP &&
		    variable[l] == DELIM)
		    return FALSE;
		goto outerspace;
	    }
	}
	return FALSE;
      outerspace:;
    }
    return TRUE;
}					/* end of kill_ok */
/******************************************************************************/
void
lineref (adrr)				/* parse lineref and return pos.in routine */
	char  **adrr;			/* result: [pointer to] pointer to line */

{
    long    offset,
            j;
    char   *reg,
           *beg;

    while (*codptr == '@') {		/* handle indirection */
	codptr++;
	expr (ARGIND);
	if (ierr > 0)
	    return;
	stcat (argptr, codptr);
	stcpy (code, argptr);
	codptr = code;
    }

    offset = 0;
    beg = rouptr;
    if (*codptr == '+') {
	codptr++;
	expr (STRING);
	if (ierr > 0)
	    return;
	if ((offset = intexpr (argptr)) <= 0) {
	    *adrr = 0;
	    return;
	}
	offset--;
    } else {
	expr (LABEL);
	if (ierr > 0)
	    return;
	reg = beg;
	while (beg < rouend) {
	    reg++;
	    if ((*reg) != TAB && (*reg) != SP) {
		j = 0;
		while ((*reg) == varnam[j]) {
		    reg++;
		    j++;
		}
		if (((*reg) == TAB ||
		     (*reg) == SP ||
		     (*reg) == '(') && varnam[j] == EOL)
		    break;
	    }
	    reg = (beg = beg + UNSIGN (*beg) + 2);
	}
	stcpy (varerr, varnam);
	varnam[0] = EOL;
	codptr++;
	if (*codptr == '+') {
	    codptr++;
	    expr (STRING);
	    if (ierr > 0)
		return;
	    offset = intexpr (argptr);
	}
    }
    if (offset < 0) {
	reg = rouptr;
	while (reg < beg) {
	    reg += UNSIGN (*reg) + 2;
	    offset++;
	}
	if (offset < 0) {
	    *adrr = 0;
	    return;
	}
	beg = rouptr;
    }
    while (offset-- > 0 && beg <= rouend)
	beg += UNSIGN (*beg) + 2;
    *adrr = beg;
    return;
}					/* end of lineref() */
/******************************************************************************/
void
zi (line, position)			/* insert 'line' in routine at 'position' */
	char   *line;
	char   *position;
{
    short   offset,
            label,
            i,
            i0,
            ch;
    char   *reg,
           *end;
    char    line0[256];

    if (rouend - rouptr + stlen (line) + 1 > PSIZE0) {	/* sufficient space ??? */
	reg = buff;
	if (getrmore () == 0L)
	    return;			/* PGMOV */
	position += buff - reg;
    }
    label = TRUE;
    i = 0;
    i0 = 0;
    while ((ch = line[i]) != EOL) {
	if (label) {
	    if (ch == SP)
		ch = TAB;
	    if (ch == TAB)
		label = FALSE;
	    else if (ch == '(') {
		line0[i0++] = ch;
		i++;
		while (((ch = line[i]) >= 'A' && ch <= 'Z') ||
		       (ch >= 'a' && ch <= 'z') ||
		       (ch >= '0' && ch <= '9') ||
		       ch == '%' || ch == ',') {
		    line0[i0++] = ch;
		    i++;
		}
		if (ch != ')') {
		    ierr = ISYNTX;
		    return;
		}
		line0[i0++] = ch;
		i++;
		if ((ch = line[i]) != SP && ch != TAB) {
		    ierr = ISYNTX;
		    return;
		}
		continue;
	    } else if ((ch < 'a' || ch > 'z') && (ch < 'A' || ch > 'Z') &&
		       (ch < '0' || ch > '9') && (ch != '%' || i)) {
		ierr = ISYNTX;
		return;
	    }
	    line0[i0++] = ch;
	    i++;
	    continue;
	}
	if (ch < SP || (ch >= DEL && (eightbit == FALSE))) {
	    ierr = ISYNTX;
	    return;
	}
	line0[i0++] = ch;
	i++;
    }
    if (label) {
	ierr = ISYNTX;
	return;
    }
    line0[i0] = EOL;
    offset = i0;
    if (offset > 0) {
	offset += 2;
	end = rouend;
	rouend += offset;
	if (roucur > position || roucur > end)
	    roucur += offset;
	reg = rouend;
	while (position <= end) {
	    (*reg--) = (*end--);
	}
	(*(position++)) = (UNSIGN (offset) - 2);
	reg = line0;
	while (((*(position++)) = (*(reg++))) != EOL) ;
	*(rouend + 1) = EOL;
	*(rouend + 2) = EOL;
	for (i = 0; i < NO_OF_RBUF; i++) {
	    if (rouptr == (buff + (i * PSIZE0))) {
		ends[i] = rouend;
		break;
	    }
	}
    }
    rouins = position;
    return;
}					/* end of zi() */
/******************************************************************************/
void
write_f (intext)
	char   *intext;			/* write this format */

{
    char    outtext[256];		/* output string */
    short   l;
    int     i;
    char    final;
    int     csi;

    csi = FALSE;
    l = stlen (intext);
    if (l < 2)
	return;				/* not recognized */
    for (i = 0; i < l; i++)
	if (intext[i] == DELIM)
	    break;
    intext[i] = EOL;
/* CUB - Cursor Backward        */
    if (!stcmp (intext, "CUB\201")) {
	csi = TRUE;
	final = 'D';
	goto end;
    }
/* CUD - Cursor Down            */
    if (!stcmp (intext, "CUD\201")) {
	csi = TRUE;
	final = 'B';
	goto end;
    }
/* CUF - Cursor Forward         */
    if (!stcmp (intext, "CUF\201")) {
	csi = TRUE;
	final = 'C';
	goto end;
    }
/* CUP - Cursor Position        */
    if (!stcmp (intext, "CUP\201") ||
/* HVP - Horizontal and vertical Position */
	!stcmp (intext, "HVP\201")) {
	csi = TRUE;
	final = 'H';
	goto end;
    }
/* CUU - Cursor Up              */
    if (!stcmp (intext, "CUU\201")) {
	csi = TRUE;
	final = 'A';
	goto end;
    }
/* DCH - Delete Character */
    if (!stcmp (intext, "DCH\201")) {
	csi = TRUE;
	final = 'P';
	goto end;
    }
/* ICH - Insert Character */
    if (!stcmp (intext, "ICH\201")) {
	csi = TRUE;
	final = '@';
	goto end;
    }
/* DL  - Delete Line */
    if (!stcmp (intext, "DL\201")) {
	csi = TRUE;
	final = 'M';
	goto end;
    }
/* IL  - Insert Line */
    if (!stcmp (intext, "IL\201")) {
	csi = TRUE;
	final = 'L';
	goto end;
    }
/* SU  - Scroll Up = pan down */
    if (!stcmp (intext, "SU\201")) {
	csi = TRUE;
	final = 'S';
	goto end;
    }
/* SD  - Scroll Down = pan up */
    if (!stcmp (intext, "SD\201")) {
	csi = TRUE;
	final = 'T';
	goto end;
    }
/* DA  - Device Attributes      */
    if (!stcmp (intext, "DA\201")) {
	csi = TRUE;
	final = 'c';
	goto end;
    }
/* DSR - Device Status Report   */
    if (!stcmp (intext, "DSR\201")) {
	csi = TRUE;
	final = 'n';
	goto end;
    }
/* ED  - Erase Display          */
    if (!stcmp (intext, "ED\201")) {
	csi = TRUE;
	final = 'J';
	goto end;
    }
/* EL  - Erase Line             */
    if (!stcmp (intext, "EL\201")) {
	csi = TRUE;
	final = 'K';
	goto end;
    }
/* ECH - Erase Character */
    if (!stcmp (intext, "ECH\201")) {
	csi = TRUE;
	final = 'X';
	goto end;
    }
/* HTS - Horizontal Tabulation Set */
    if (!stcmp (intext, "HTS\201")) {
	final = 'H';
	goto end;
    }
/* IND - Index                  */
    if (!stcmp (intext, "IND\201")) {
	final = 'D';
	goto end;
    }
/* NEL - NExt Line              */
    if (!stcmp (intext, "NEL\201")) {
	final = 'E';
	goto end;
    }
    if (!stcmp (intext, "SSA\201")) {
	final = 'F';
	goto end;
    }
    if (!stcmp (intext, "ESA\201")) {
	final = 'G';
	goto end;
    }
    if (!stcmp (intext, "HTJ\201")) {
	final = 'I';
	goto end;
    }
    if (!stcmp (intext, "VTS\201")) {
	final = 'J';
	goto end;
    }
    if (!stcmp (intext, "PLD\201")) {
	final = 'K';
	goto end;
    }
    if (!stcmp (intext, "PLU\201")) {
	final = 'L';
	goto end;
    }
/* RI  - Reverse Index          */
    if (!stcmp (intext, "RI\201")) {
	final = 'M';
	goto end;
    }
/* SS2 - Single Shift G2 */
    if (!stcmp (intext, "SS2\201")) {
	final = 'N';
	goto end;
    }
/* SS3 - Single Shift G3 */
    if (!stcmp (intext, "SS3\201")) {
	final = 'O';
	goto end;
    }
/* DCS - Device Control String introducer */
    if (!stcmp (intext, "DCS\201")) {
	final = 'P';
	goto end;
    }
    if (!stcmp (intext, "PU1\201")) {
	final = 'Q';
	goto end;
    }
    if (!stcmp (intext, "PU2\201")) {
	final = 'R';
	goto end;
    }
    if (!stcmp (intext, "STS\201")) {
	final = 'S';
	goto end;
    }
    if (!stcmp (intext, "CCH\201")) {
	final = 'T';
	goto end;
    }
    if (!stcmp (intext, "MW\201")) {
	final = 'U';
	goto end;
    }
    if (!stcmp (intext, "SPA\201")) {
	final = 'V';
	goto end;
    }
    if (!stcmp (intext, "EPA\201")) {
	final = 'W';
	goto end;
    }
/* CSI - Command String Introducer */
    if (!stcmp (intext, "CSI\201")) {
	final = '[';
	goto end;
    }
/* ST - device control String Terminator */
    if (!stcmp (intext, "ST\201")) {
	final = '\\';
	goto end;
    }
    if (!stcmp (intext, "OSC\201")) {
	final = ']';
	goto end;
    }
    if (!stcmp (intext, "PM\201")) {
	final = '^';
	goto end;
    }
    if (!stcmp (intext, "APC\201")) {
	final = '_';
	goto end;
    }
/* RIS - Reset to Initial State */
    if (!stcmp (intext, "RIS\201")) {
	final = 'c';
	goto end;
    }
/* RM  - Reset Mode             */
    if (!stcmp (intext, "RM\201")) {
	csi = TRUE;
	final = 'l';
	goto end;
    }
/* SGR - Select Graphic Rendition */
    if (!stcmp (intext, "SGR\201")) {
	csi = TRUE;
	final = 'm';
	goto end;
    }
/* SM  - Set Mode               */
    if (!stcmp (intext, "SM\201")) {
	csi = TRUE;
	final = 'h';
	goto end;
    }
/* TBC - Tabulation Clear       */
    if (!stcmp (intext, "TBC\201")) {
	csi = TRUE;
	final = 'g';
	goto end;
    }
    if (!stcmp (intext, "NUL\201")) {
	final = NUL;
	goto controls;
    }
    if (!stcmp (intext, "SOH\201")) {
	final = SOH;
	goto controls;
    }
    if (!stcmp (intext, "STX\201")) {
	final = STX;
	goto controls;
    }
    if (!stcmp (intext, "ETX\201")) {
	final = ETX;
	goto controls;
    }
    if (!stcmp (intext, "EOT\201")) {
	final = EOT;
	goto controls;
    }
    if (!stcmp (intext, "ENQ\201")) {
	final = ENQ;
	goto controls;
    }
    if (!stcmp (intext, "ACK\201")) {
	final = ACK;
	goto controls;
    }
    if (!stcmp (intext, "BEL\201")) {
	final = BEL;
	goto controls;
    }
    if (!stcmp (intext, "BS\201")) {
	final = BS;
	goto controls;
    }
    if (!stcmp (intext, "HT\201")) {
	final = TAB;
	goto controls;
    }
    if (!stcmp (intext, "LF\201")) {
	final = LF;
	goto controls;
    }
    if (!stcmp (intext, "VT\201")) {
	final = VT;
	goto controls;
    }
    if (!stcmp (intext, "FF\201")) {
	final = FF;
	goto controls;
    }
    if (!stcmp (intext, "CR\201")) {
	final = CR;
	goto controls;
    }
    if (!stcmp (intext, "SO\201")) {
	final = SO;
	goto controls;
    }
    if (!stcmp (intext, "SI\201")) {
	final = SI;
	goto controls;
    }
    if (!stcmp (intext, "DLE\201")) {
	final = DLE;
	goto controls;
    }
    if (!stcmp (intext, "DC1\201")) {
	final = DC1;
	goto controls;
    }
    if (!stcmp (intext, "DC2\201")) {
	final = DC2;
	goto controls;
    }
    if (!stcmp (intext, "DC3\201")) {
	final = DC3;
	goto controls;
    }
    if (!stcmp (intext, "DC4\201")) {
	final = DC4;
	goto controls;
    }
    if (!stcmp (intext, "NAK\201")) {
	final = NAK;
	goto controls;
    }
    if (!stcmp (intext, "SYN\201")) {
	final = SYN;
	goto controls;
    }
    if (!stcmp (intext, "ETB\201")) {
	final = ETB;
	goto controls;
    }
    if (!stcmp (intext, "CAN\201")) {
	final = CAN;
	goto controls;
    }
    if (!stcmp (intext, "EM\201")) {
	final = EM;
	goto controls;
    }
    if (!stcmp (intext, "SUB\201")) {
	final = SUB;
	goto controls;
    }
    if (!stcmp (intext, "ESC\201")) {
	final = ESC;
	goto controls;
    }
    if (!stcmp (intext, "FS\201")) {
	final = FS;
	goto controls;
    }
    if (!stcmp (intext, "GS\201")) {
	final = GS;
	goto controls;
    }
    if (!stcmp (intext, "RS\201")) {
	final = RS;
	goto controls;
    }
    if (!stcmp (intext, "US\201")) {
	final = US;
	goto controls;
    }
    if (!stcmp (intext, "DEL\201")) {
	final = DEL;
	goto controls;
    }
/* DECKPAM Keypad Application Mode */
    if (!stcmp (intext, "DECKPAM\201")) {
	final = '=';
	goto end;
    }
/* DECKPNM Keypad Numeric Mode  */
    if (!stcmp (intext, "DECKPNM\201")) {
	final = '>';
	goto end;
    }
/* DECLL Load LEDs              */
    if (!stcmp (intext, "DECLL\201")) {
	csi = TRUE;
	final = 'q';
	goto end;
    }
/* DECRC Restore Cursor         */
    if (!stcmp (intext, "DECRC\201")) {
	final = '8';
	goto end;
    }
/* DECSC Save Cursor            */
    if (!stcmp (intext, "DECSC\201")) {
	final = '7';
	goto end;
    }
/* DECSTBM Set Top & Bottom Margins */
    if (!stcmp (intext, "TBM\201") ||
	!stcmp (intext, "DECSTBM\201")) {
	csi = TRUE;
	final = 'r';
	goto end;
    }
/* ZAS Alternate Screen */
    if (!stcmp (intext, "ZAS\201")) {
	csi = TRUE;
	final = '~';
	goto end;
    }
    return;				/* code not recognized */

  controls:
    outtext[0] = final;
    outtext[1] = EOL;
    write_m (outtext);
    return;

  end:
    outtext[0] = ESC;
    if (csi++)
	outtext[1] = '[';
    while (++i < l) {
	if ((outtext[csi] = intext[i]) == DELIM)
	    outtext[csi] = ';';
	csi++;
    }
    outtext[csi++] = final;
    outtext[csi] = EOL;
    write_m (outtext);
    return;
}					/* end of write_f() */
/******************************************************************************/
void
writeHOME (text)			/* Output on HOME device */
	char   *text;			/* write this string */

{
    static char initflag = TRUE;	/* initialisation flag */
    static char esc = 0;		/* esc processing flag */
    static char dcs = 0;		/* device control processing flag */

    static short args[ARGS_IN_ESC];
    static short argcnt = 0;
    static short noargs = TRUE;
    static char tmp[512];
    static short foreground = TRUE;	/* foreground flag */
    static struct vtstyp *vts;

/* external struct vtstyp *screen;         active screen */
    static struct vtstyp *altscr;	/* background screen */
    static struct vtstyp *zases[MAXZAS];
    static int zaslevel = 0;

/* SGR and CSI=cF have ***different*** color codes */
#ifdef COLOR
    static char coltrans[8] =
    {0, 4, 2, 6, 1, 5, 3, 7};

#endif /* COLOR */


    short   tmpx;
    register ch;
    register j;
    register i;
    short   k;

/* we assume the HOME device is an ASCII machine according
 * to ANSI X3.4, X3.64 etc with 24 lines and 80 columns
 * so we look not only at controls as the MUMPS X11.1-1984
 * demands, but as well at esc sequences and device control
 * strings.
 * 
 * In addition we assume, that the terminal cannot handle
 * tab_clear (CSI 3g) nor tab_set (ESC H) so these functions
 * are emulated here. For most terminals that might
 * not be neccesary. With 'PROC_TAB' we may switch versions.
 * 
 * We support the VT100 on PC-"Terminals"
 * where there is no TBM so we have to emulate
 * scoll up /down with LF,RI and autowrap. SGR is somewhat
 * crazy on the PC and there is a lot of code to fix that.
 * The PC completely ignores SGRs it does not fully recognize.
 * E.g. SGR(7,4) will not work while SGR(4,7) at least inverts
 * the display.
 * CSI 10 p (invert INVERS at active position) 
 * is being emulated too.
 * We emulate the terminal software so we have always an image
 * of the screen in memory. That enables us to have features like
 * CSI 0 ~  (private sequence: change screen)
 * CSI 1 ~  (private sequence: output to foreground screen)
 * CSI 2 ~  (private sequence: output to background screen)
 * CSI 3 ~  (private sequence: save foreground to background)
 * CSI 4 ~  (private sequence: screen restore)
 * and the 'hardcopy function'
 */
    if (initflag) {
	screen = (struct vtstyp *) calloc (1, sizeof (struct vtstyp));

	ris (screen);
	altscr = (struct vtstyp *) calloc (1, sizeof (struct vtstyp));

	ris (altscr);
	initflag = FALSE;
    }
    opnfile[HOME] = stdout;
    tmpx = 0;
    j = 0;
    while ((ch = text[j++]) != EOL) {
	if (ch == NUL)
	    continue;
	if (tmpx > 480) {
	    tmp[tmpx] = EOL;
	    tmpx = 0;
	    if (foreground)
		m_output (tmp);
	}
	if (RightMargin && xpos[HOME] > RightMargin && esc == 0) {
	    --j;
	    tmp[tmpx++] = CR;
	    xpos[HOME] = 0;
	    ch = LF;
	}
#ifdef PROC_TAB
	if (ch != TAB)
#endif /* PROC_TAB */
	    tmp[tmpx++] = ch;
	if (UNSIGN (ch) >= SP && ch != DEL) {	/* printable characters */
	    if (esc == 0) {		/* no esc/dcs in progress; wrap-around */
		(*screen).screenx[(unsigned int) (*screen).sclines[ypos[HOME]]][xpos[HOME]] = ch;
		if ((*screen).savarg == FALSE) {
#ifdef COLOR
		    (*screen).screenc[(unsigned int) (*screen).sclines[ypos[HOME]]][xpos[HOME]] = (*screen).col;
#endif /* COLOR */
		    (*screen).screena[(unsigned int) (*screen).sclines[ypos[HOME]]][xpos[HOME]] = (*screen).att;
		}
		if (dcs == 0 && ++xpos[HOME] >= N_COLUMNS) {
		    xpos[HOME] = 0;
		    if ((*screen).rollflag)
			goto pLF;
		    ypos[HOME] = (*screen).sc_lo;
		}
		continue;
	    }
	    if (esc == 1) {		/* esc_sequence in progress */
		if (ch == '[') {	/* CSI command string starts */
		    esc = 2;
		    continue;
		} else if (ch == 'H') {	/* HTS set tab at $X */
		    (*screen).tabs[xpos[HOME]] = 1;
		} else if (ch == 'M') {	/* RI cursor up *//* upper margin of scroll area: scroll down */
		    if (ypos[HOME] == (*screen).sc_up) {
			k = (*screen).sclines[(*screen).sc_lo];
			for (i = (*screen).sc_lo; i > (*screen).sc_up; i--)
			    (*screen).sclines[i] = (*screen).sclines[i - 1];
			(*screen).sclines[(*screen).sc_up] = k;
			for (i = 0; i < N_COLUMNS; i++) {
			    (*screen).screenx[k][i] = SP;
			    (*screen).screena[k][i] = (*screen).att;
#ifdef COLOR
			    (*screen).screenc[k][i] = (*screen).col;
#endif /* COLOR */
			}
			if (foreground) {
			    tmp[tmpx - 2] = EOL;
			    m_output (tmp);
			    tmpx = 0;
			    part_ref (screen, (*screen).sc_up, (*screen).sc_lo);
			}
		    }
		    if (ypos[HOME] != 0 && ypos[HOME] != (*screen).sc_up)
			ypos[HOME]--;
		    tmp[tmpx++] = ESC;
		    tmp[tmpx++] = '[';
		    if (ypos[HOME] > 8)
			tmp[tmpx++] = (1 + ypos[HOME]) / 10 + '0';
		    tmp[tmpx++] = (1 + ypos[HOME]) % 10 + '0';
		    if (xpos[HOME] > 0) {
			tmp[tmpx++] = ';';
			if (xpos[HOME] > 8)
			    tmp[tmpx++] = (1 + xpos[HOME]) / 10 + '0';
			tmp[tmpx++] = (1 + xpos[HOME]) % 10 + '0';
		    }
		    tmp[tmpx++] = 'H';
		}
/* RI */
		else if (ch == 'E') {	/* NEL new line */
		    xpos[HOME] = 0;
		    esc = 0;
		    goto pLF;
		} else if (ch == 'Q') {	/* DCS Device control string */
		    dcs = 1;
		} else if (ch == '\\') {	/* ST String terminator (DCS) */
		    dcs = 0;
		} else if (ch == '7') {	/* DEC CS Cursor Save */
		    (*screen).csx[(*screen).cs] = xpos[HOME];
		    (*screen).csy[(*screen).cs] = ypos[HOME];
		    if (++((*screen).cs) >= CSLEN)
			(*screen).cs = CSLEN - 1;
		} else if (ch == '8') {	/* DEC CRST Cursor Restore */
		    if (--((*screen).cs) <= 0)
			(*screen).cs = 0;
		    xpos[HOME] = (*screen).csx[(*screen).cs];
		    ypos[HOME] = (*screen).csy[(*screen).cs];
/* make sure cursor is at desired position */
		    tmp[tmpx++] = ESC;
		    tmp[tmpx++] = '[';
		    if (ypos[HOME] > 8)
			tmp[tmpx++] = (1 + ypos[HOME]) / 10 + '0';
		    tmp[tmpx++] = (1 + ypos[HOME]) % 10 + '0';
		    if (xpos[HOME] > 0) {
			tmp[tmpx++] = ';';
			if (xpos[HOME] > 8)
			    tmp[tmpx++] = (1 + xpos[HOME]) / 10 + '0';
			tmp[tmpx++] = (1 + xpos[HOME]) % 10 + '0';
		    }
		    tmp[tmpx++] = 'H';
		} else if (ch == 'c') {	/* RIS Reset to initial state */
		    esc = 0;
		    dcs = 0;
		    foreground = TRUE;
		    xpos[HOME] = 0;
		    ypos[HOME] = 0;
		    ris (screen);
		} else if (ch == '(' || ch == ')')
		    continue;
		esc = 0;
		continue;
	    }				/* end if (esc==1) */
/* command string (esc [) in progress */
/* numeric arguments ? */
	    if (ch >= '0' && ch <= '9') {
		noargs = FALSE;
		args[argcnt] = args[argcnt] * 10 + ch - '0';
		continue;
	    }
	    if (ch == ';') {
		args[++argcnt] = 0;
		continue;
	    }
	    if (ch == '=') {
		esc = 3;
		continue;
	    }				/* color sequence */
	    if (esc == 3) {
		esc = 0;
#ifdef COLOR
		if (ch == 'F') {	/* foreground color */
		    (*screen).col = (((*screen).col & ~017) | (args[0] & 017));
		    tmp[tmpx++] = ESC;	/* reverse background */
		    tmp[tmpx++] = '[';
		    tmp[tmpx++] = '=';
		    if (args[0] > 9)
			tmp[tmpx++] = '0' + (args[0] / 10);
		    tmp[tmpx++] = '0' + (args[0] % 10);
		    tmp[tmpx++] = 'I';
		    continue;
		}
		if (ch == 'G') {	/* background color */
		    (*screen).col = (((*screen).col & 017) | (args[0] * 16));
		    tmp[tmpx++] = ESC;	/* reverse forground */
		    tmp[tmpx++] = '[';
		    tmp[tmpx++] = '=';
		    if (args[0] > 9)
			tmp[tmpx++] = '0' + (args[0] / 10);
		    tmp[tmpx++] = '0' + (args[0] % 10);
		    tmp[tmpx++] = 'H';
		    continue;
		}
#endif /* COLOR */
		continue;
	    }
	    if (ch < '@')
		continue;
/* check final characters */
	    if (ch == 'A') {		/* CUU cursor up */
		do
		    if (--ypos[HOME] < 0)
			ypos[HOME] = 0;
		while (--args[0] > 0) ;
	    } else if (ch == 'B') {	/* CUD cursor down */
		do
		    if (++ypos[HOME] >= (N_LINES - 1))
			ypos[HOME] = (N_LINES - 1);
		while (--args[0] > 0) ;
	    } else if (ch == 'C') {	/* CUF cursor right */
		do
		    if (++xpos[HOME] >= N_COLUMNS)
			xpos[HOME] = (N_COLUMNS - 1);
		while (--args[0] > 0) ;
	    } else if (ch == 'D') {	/* CUB cursor left */
		do
		    if (--xpos[HOME] < 0)
			xpos[HOME] = 0;
		while (--args[0] > 0) ;
	    } else if (ch == 'H') {	/* CUP cursor position */
		i = --args[0];
		if (i < 0)
		    i = 0;
		if (i <= N_LINES)
		    ypos[HOME] = i;
		i = --args[1];
		if (i < 0)
		    i = 0;
		if (i < N_COLUMNS)
		    xpos[HOME] = i;
	    } else if (ch == 'K') {	/* EL erase line */
		int     k;
		short   xatt;

		i = 0;
		k = N_COLUMNS;
		if (args[0] == 0)
		    i = xpos[HOME];
		if (args[0] == 1)
		    k = xpos[HOME] + 1;
		while (i < k) {
		    (*screen).screenx[(unsigned int) (*screen).sclines[ypos[HOME]]][i] = SP;
		    (*screen).screena[(unsigned int) (*screen).sclines[ypos[HOME]]][i] = 0;	/* not 'att' */
#ifdef COLOR
		    (*screen).screenc[(unsigned int) (*screen).sclines[ypos[HOME]]][i] = (*screen).col;
#endif /* COLOR */
		    i++;
		}
#ifdef SCO
#endif /* SCO */
	    } else
	  ED:if (ch == 'J') {		/* ED erase display */
		register k;
		unsigned char xatt;

#ifdef COLOR
		unsigned char color;

#endif /* COLOR */
		i = 0;
		k = N_COLUMNS;
		if (args[0] == 0)
		    i = xpos[HOME];
		if (args[0] == 1)
		    k = xpos[HOME] + 1;
#ifdef COLOR
		color = (*screen).col;
#endif /* COLOR */
		while (i < k) {		/* clear current line */
		    (*screen).screenx[(unsigned int) (*screen).sclines[ypos[HOME]]][i] = SP;
		    (*screen).screena[(unsigned int) (*screen).sclines[ypos[HOME]]][i] = 0;	/* not 'att' */
#ifdef COLOR
		    (*screen).screenc[(unsigned int) (*screen).sclines[ypos[HOME]]][i] = color;
#endif /* COLOR */
		    i++;
		}
		if (args[0] != 1)	/* clear rest of screen */
		    for (k = ypos[HOME] + 1; k < N_LINES; k++)
			for (i = 0; i < N_COLUMNS; i++) {
			    (*screen).screenx[(unsigned int) (*screen).sclines[k]][i] = SP;
			    (*screen).screena[(unsigned int) (*screen).sclines[k]][i] = 0;
#ifdef COLOR
			    (*screen).screenc[(unsigned int) (*screen).sclines[k]][i] = color;
#endif /* COLOR */
			}
		if (args[0] != 0)	/* clear begin of screen */
		    for (k = 0; k < ypos[HOME]; k++)
			for (i = 0; i < N_COLUMNS; i++) {
			    (*screen).screenx[(unsigned int) (*screen).sclines[k]][i] = SP;
			    (*screen).screena[(unsigned int) (*screen).sclines[k]][i] = 0;
#ifdef COLOR
			    (*screen).screenc[(unsigned int) (*screen).sclines[k]][i] = color;
#endif /* COLOR */
			}
#ifdef SCO
		if (foreground) {
		    if ((*screen).lin24)
			tmp[tmpx - 1] = CAN;
		    tmp[tmpx] = EOL;
		    m_output (tmp);
		    tmpx = 0;
		    part_ref (screen, 0, N_LINES - 1);
		}
#endif /* SCO */
	    }
/* end ED */
	    else if (ch == 'p') {	/* private */
		if (args[0] == 10) {	/* invert 'inverse' at */
/* current cursor pos. */ (*screen).screena[(unsigned int) (*screen).sclines[ypos[HOME]]][xpos[HOME]] ^= 0100;
#ifdef SCO
		    if (foreground) {
			k = (*screen).screena[(unsigned int) (*screen).sclines[ypos[HOME]]][xpos[HOME]];
			if (((*screen).att & 01) != (k & 01))
			    tmp[tmpx++] = (k & 01) ? SO : SI;
			k = scosgr (k, (*screen).bw);
			tmp[tmpx++] = ESC;
			tmp[tmpx++] = '[';
			if ((*screen).bw)
			    tmp[tmpx++] = '7';
			tmp[tmpx++] = 'm';
			tmp[tmpx++] = ESC;
			tmp[tmpx++] = '[';
			if (k & 02) {
			    tmp[tmpx++] = '1';
			    tmp[tmpx++] = ';';
			}		/* SCO !!! */
			if (k & 04) {
			    tmp[tmpx++] = '3';
			    tmp[tmpx++] = ';';
			}
			if (k & 010) {
			    tmp[tmpx++] = '4';
			    tmp[tmpx++] = ';';
			}
			if (k & 020) {
			    tmp[tmpx++] = '5';
			    tmp[tmpx++] = ';';
			}
			if (k & 040) {
			    tmp[tmpx++] = '6';
			    tmp[tmpx++] = ';';
			}
			if (k & 0100) {
			    tmp[tmpx++] = '7';
			    tmp[tmpx++] = ';';
			}
			if (k & 0200) {
			    tmp[tmpx++] = '8';
			    tmp[tmpx++] = ';';
			}
			if (tmp[tmpx - 1] == ';')
			    tmpx--;
			tmp[tmpx++] = 'm';
			tmp[tmpx++] = (*screen).screenx[(unsigned int) (*screen).sclines[ypos[HOME]]][xpos[HOME]];
			tmp[tmpx++] = ESC;
			tmp[tmpx++] = '[';
			k = ypos[HOME] + 1;
			if (k > 9)
			    tmp[tmpx++] = k / 10 + '0';
			tmp[tmpx++] = k % 10 + '0';
			if (xpos[HOME]) {
			    tmp[tmpx++] = ';';
			    k = xpos[HOME] + 1;
			    if (k > 9)
				tmp[tmpx++] = k / 10 + '0';
			    tmp[tmpx++] = k % 10 + '0';
			}
			tmp[tmpx++] = 'H';
			if (k != screen->att) {
			    k = scosgr ((*screen).att, (*screen).bw);
			    tmp[tmpx++] = (k & 01) ? SO : SI;
			    tmp[tmpx++] = ESC;
			    tmp[tmpx++] = '[';
			    tmp[tmpx++] = '0';
			    if (k & 02) {
				tmp[tmpx++] = ';';
				tmp[tmpx++] = '1';
			    }		/* SCO !!! */
			    if (k & 04) {
				tmp[tmpx++] = ';';
				tmp[tmpx++] = '3';
			    }
			    if (k & 010) {
				tmp[tmpx++] = ';';
				tmp[tmpx++] = '4';
			    }
			    if (k & 020) {
				tmp[tmpx++] = ';';
				tmp[tmpx++] = '5';
			    }
			    if (k & 040) {
				tmp[tmpx++] = ';';
				tmp[tmpx++] = '6';
			    }
			    if (k & 0100) {
				tmp[tmpx++] = ';';
				tmp[tmpx++] = '7';
			    }
			    if (k & 0200) {
				tmp[tmpx++] = ';';
				tmp[tmpx++] = '8';
			    }
			    tmp[tmpx++] = 'm';
			}
		    }
#endif /* SCO */

		}
	    } else if (ch == 'r') {	/* TBM Top_Bottom Margin */
		register k;

		if (noargs ||
		    ((*screen).sc_up = (--args[0])) <= 0)
		    (*screen).sc_up = 0;
		if (!argcnt ||
		    (((*screen).sc_lo = (--args[1])) > 24) ||
		    ((*screen).sc_lo == 23 && (*screen).lin24 == TRUE))
		    (*screen).sc_lo = (*screen).lin24 ? 23 : 24;
/* restore old cursor position */
		tmp[tmpx++] = ESC;
		tmp[tmpx++] = '[';
		k = ypos[HOME] + 1;
		if (k > 9)
		    tmp[tmpx++] = k / 10 + '0';
		tmp[tmpx++] = k % 10 + '0';
		if (xpos[HOME]) {
		    tmp[tmpx++] = ';';
		    k = xpos[HOME] + 1;
		    if (k > 9)
			tmp[tmpx++] = k / 10 + '0';
		    tmp[tmpx++] = k % 10 + '0';
		}
		tmp[tmpx++] = 'H';
	    } else if (ch == 's') {	/* CS Cursor Save */
		(*screen).csx[(*screen).cs] = xpos[HOME];
		(*screen).csy[(*screen).cs] = ypos[HOME];
		(*screen).cssgr[(*screen).cs] = (*screen).att;
#ifdef COLOR
		(*screen).cscol[(*screen).cs] = (*screen).col;
#endif /* COLOR */
		if (++((*screen).cs) >= CSLEN)
		    (*screen).cs = CSLEN - 1;
	    } else if (ch == 'u') {	/* CRST Cursor Unsave (if no args) */
/* those bloody bastards made 'CSI u' and CSI 0 u' */
/* different. flag 'noargs' distinguishes          */
		if (noargs) {
#ifdef COLOR
		    int     color;

#endif /* COLOR */
		    if (--((*screen).cs) <= 0)
			(*screen).cs = 0;
		    xpos[HOME] = (*screen).csx[(*screen).cs];
		    ypos[HOME] = (*screen).csy[(*screen).cs];
		    (*screen).att = (*screen).cssgr[(*screen).cs];
#ifdef COLOR
		    (*screen).col = (*screen).cscol[(*screen).cs];
		    color = (*screen).col;
#endif /* COLOR */
/* make sure cursor is at desired position */
		    tmp[tmpx++] = ESC;
		    tmp[tmpx++] = '[';
		    if (ypos[HOME] > 8)
			tmp[tmpx++] = (1 + ypos[HOME]) / 10 + '0';
		    tmp[tmpx++] = (1 + ypos[HOME]) % 10 + '0';
		    if (xpos[HOME] > 0) {
			tmp[tmpx++] = ';';
			if (xpos[HOME] > 8)
			    tmp[tmpx++] = (1 + xpos[HOME]) / 10 + '0';
			tmp[tmpx++] = (1 + xpos[HOME]) % 10 + '0';
		    }
		    tmp[tmpx++] = 'H';
/* make sure cursor has right color */
#ifdef COLOR
		    tmp[tmpx++] = ESC;	/* background color */
		    tmp[tmpx++] = '[';
		    tmp[tmpx++] = '=';
		    if (color / 16 > 9)
			tmp[tmpx++] = '0' + (color / 16 / 10);
		    tmp[tmpx++] = '0' + (color / 16 % 10);
		    tmp[tmpx++] = 'G';
		    tmp[tmpx++] = ESC;	/* reverse foreground col */
		    tmp[tmpx++] = '[';
		    tmp[tmpx++] = '=';
		    if (color / 16 > 9)
			tmp[tmpx++] = '0' + (color / 16 / 10);
		    tmp[tmpx++] = '0' + (color / 16 % 10);
		    tmp[tmpx++] = 'H';
		    tmp[tmpx++] = ESC;	/* foreground color */
		    tmp[tmpx++] = '[';
		    tmp[tmpx++] = '=';
		    if (color % 16 > 9)
			tmp[tmpx++] = '0' + (color % 16 / 10);
		    tmp[tmpx++] = '0' + (color % 16 % 10);
		    tmp[tmpx++] = 'F';
		    tmp[tmpx++] = ESC;	/* reverse background col */
		    tmp[tmpx++] = '[';
		    tmp[tmpx++] = '=';
		    if (color % 16 > 9)
			tmp[tmpx++] = '0' + (color % 16 / 10);
		    tmp[tmpx++] = '0' + (color % 16 % 10);
		    tmp[tmpx++] = 'I';
#endif /* COLOR */
		} else if (args[0] == 0) {
		    (*screen).lin24 = FALSE;
		    if ((*screen).sc_lo == 23)
			(*screen).sc_lo = 24;
		} else if (args[0] == 1) {
		    (*screen).lin24 = TRUE;
		    if ((*screen).sc_lo == 24)
			(*screen).sc_lo = 23;
		} else if (args[0] == 8)
		    (*screen).rollflag = FALSE;
		else if (args[0] == 9)
		    (*screen).rollflag = TRUE;
#ifdef SCO
		else if (args[0] == 20) {
		    (*screen).bw = FALSE;
		    if (foreground) {
			tmp[tmpx] = EOL;
			m_output (tmp);
			tmpx = 0;
			part_ref (screen, 0, N_LINES - 1);
		    }
		} else if (args[0] == 21) {
		    (*screen).bw = TRUE;
		    if (foreground) {
			tmp[tmpx] = EOL;
			m_output (tmp);
			tmpx = 0;
			part_ref (screen, 0, N_LINES - 1);
		    }
		}
#endif /* SCO */
	    } else if (ch == 'X') {	/* ECH Erase Character */
		if ((k = args[0]) < 1)
		    k = 1;
		if ((k + xpos[HOME]) > N_COLUMNS)
		    k = N_COLUMNS - xpos[HOME];

		i = xpos[HOME];
		k = i + k;
		while (i < k) {
		    (*screen).screenx[(unsigned int) (*screen).sclines[ypos[HOME]]][i] = SP;
		    (*screen).screena[(unsigned int) (*screen).sclines[ypos[HOME]]][i] = 0;	/* not 'att' */
#ifdef COLOR
		    (*screen).screenc[(unsigned int) (*screen).sclines[ypos[HOME]]][i] = (*screen).col;
#endif /* COLOR */
		    i++;
		}
	    } else if (ch == 'g') {	/* TBC Tabulation clear */
		if (args[0] == 3) {	/* clear all */
		    for (i = 0; i < N_COLUMNS; (*screen).tabs[i++] = 0) ;
		} else if (args[0] == 0)	/* clear one */
		    (*screen).tabs[xpos[HOME]] = 0;
#ifdef SCO
		while (tmpx >= 0) {
		    if (tmp[--tmpx] == ESC)
			break;
		}
		tmp[tmpx] = NUL;
		ch = NUL;
#endif /* SCO */
	    } else if (ch == 'm') {	/* SGR Select Graphic Rendition */
		(*screen).att &= 01;	/* clear att */
#ifdef SCO
		while (tmpx > 0 && tmp[--tmpx] != ESC) ;
		ch = CAN;
#endif /* SCO */
		i = argcnt;
		while (i >= 0) {
		    if (((*screen).savarg = (args[i] == 50)))
			continue;
#ifdef SCO				/* process save SGR(1) in position of SGR(2) */
		    if (args[i] == 1)
			args[i] = 2;
#endif /* SCO */
#ifdef COLOR
		    if (args[i] > 29 && args[i] < 38) {		/* foregr.color */
			(*screen).col =
				((*screen).col & ~017) | coltrans[args[i] - 30];
		    }
		    if (args[i] > 39 && args[i] < 48) {		/* backgr.color */
			(*screen).col =
				(((*screen).col & 017) | (coltrans[args[i] - 40] * 16));
		    }
#endif /* COLOR */
		    if (args[i] > 1 && args[i] < 9)
			(*screen).att |= (1 << (args[i] - 1));
		    if (args[i] == 0)
			(*screen).att &= 01;	/* clear att */
		    i--;
		}
#ifdef SCO
		tmp[tmpx++] = ESC;
		tmp[tmpx++] = '[';
		if ((*screen).bw)
		    tmp[tmpx++] = '7';
		tmp[tmpx++] = 'm';
#ifdef COLOR

		tmp[tmpx++] = ESC;
		tmp[tmpx++] = '[';
		tmp[tmpx++] = '=';
		if ((*screen).col / 16 > 9)
		    tmp[tmpx++] = '0' + ((*screen).col / 16 / 10);
		tmp[tmpx++] = '0' + ((*screen).col / 16 % 10);
		tmp[tmpx++] = 'G';
		tmp[tmpx++] = ESC;
		tmp[tmpx++] = '[';
		tmp[tmpx++] = '=';
		if ((*screen).col / 16 > 9)
		    tmp[tmpx++] = '0' + ((*screen).col / 16 / 10);
		tmp[tmpx++] = '0' + ((*screen).col / 16 % 10);
		tmp[tmpx++] = 'H';
		tmp[tmpx++] = ESC;
		tmp[tmpx++] = '[';
		tmp[tmpx++] = '=';
		if ((*screen).col % 16 > 9)
		    tmp[tmpx++] = '0' + ((*screen).col % 16 / 10);
		tmp[tmpx++] = '0' + ((*screen).col % 16 % 10);
		tmp[tmpx++] = 'F';
		tmp[tmpx++] = ESC;
		tmp[tmpx++] = '[';
		tmp[tmpx++] = '=';
		if ((*screen).col % 16 > 9)
		    tmp[tmpx++] = '0' + ((*screen).col % 16 / 10);
		tmp[tmpx++] = '0' + ((*screen).col % 16 % 10);
		tmp[tmpx++] = 'I';
#endif /* COLOR */

		if ((*screen).att & ~01) {
		    int     j;

		    j = scosgr ((*screen).att & ~01, (*screen).bw);
		    tmp[tmpx++] = ESC;
		    tmp[tmpx++] = '[';
		    if (j & 02) {
			tmp[tmpx++] = '1';
			tmp[tmpx++] = ';';
		    }
		    if (j & 04) {
			tmp[tmpx++] = '3';
			tmp[tmpx++] = ';';
		    }
		    if (j & 010) {
			tmp[tmpx++] = '4';
			tmp[tmpx++] = ';';
		    }
		    if (j & 020) {
			tmp[tmpx++] = '5';
			tmp[tmpx++] = ';';
		    }
		    if (j & 040) {
			tmp[tmpx++] = '6';
			tmp[tmpx++] = ';';
		    }
		    if (j & 0100) {
			tmp[tmpx++] = '7';
			tmp[tmpx++] = ';';
		    }
		    if (j & 0200) {
			tmp[tmpx++] = '8';
			tmp[tmpx++] = ';';
		    }
		    if (tmp[tmpx - 1] == ';')
			tmpx--;
		    tmp[tmpx++] = 'm';
		}
#endif /* SCO */
	    } else if (ch == 'P') {	/* DCH Delete Character */
		int     j;

		if ((j = args[0]) == 0)
		    j = 1;
		k = (*screen).sclines[ypos[HOME]];
		for (i = xpos[HOME]; i < (N_COLUMNS - j); i++) {
		    (*screen).screenx[k][i] = (*screen).screenx[k][i + j];
		    (*screen).screena[k][i] = (*screen).screena[k][i + j];
#ifdef COLOR
		    (*screen).screenc[k][i] = (*screen).screenc[k][i + j];
#endif /* COLOR */
		}
		for (; i < N_COLUMNS; i++) {
		    (*screen).screenx[k][i] = SP;
		    (*screen).screena[k][i] = (*screen).att;
#ifdef COLOR
		    (*screen).screenc[k][i] = (*screen).col;
#endif /* COLOR */
		}
	    } else if (ch == '@') {	/* ICH Insert Character */
		int     j;

		if ((j = args[0]) == 0)
		    j = 1;
		k = (*screen).sclines[ypos[HOME]];
		for (i = (N_COLUMNS - 1); i >= (xpos[HOME] + j); i--) {
		    (*screen).screenx[k][i] = (*screen).screenx[k][i - j];
		    (*screen).screena[k][i] = (*screen).screena[k][i - j];
#ifdef COLOR
		    (*screen).screenc[k][i] = (*screen).screenc[k][i - j];
#endif /* COLOR */
		}
		for (; i >= xpos[HOME]; i--) {
		    (*screen).screenx[k][i] = SP;
		    (*screen).screena[k][i] = (*screen).att;
#ifdef COLOR
		    (*screen).screenc[k][i] = (*screen).col;
#endif /* COLOR */
		}
	    } else if (ch == 'M') {	/* DL Delete Line */
		int     j = args[0];

		do {
		    k = (*screen).sclines[ypos[HOME]];
		    for (i = ypos[HOME]; i < (*screen).sc_lo; i++)
			(*screen).sclines[i] = (*screen).sclines[i + 1];
		    (*screen).sclines[i] = k;
		    for (i = 0; i < N_COLUMNS; i++) {
			(*screen).screenx[k][i] = SP;
			(*screen).screena[k][i] = (*screen).att;
#ifdef COLOR
			(*screen).screenc[k][i] = (*screen).col;
#endif /* COLOR */
		    }
		} while (--j > 0);
		xpos[HOME] = 0;
#ifdef SCO
		if (foreground) {
		    tmp[tmpx - 1] = CAN;	/* do not send DL */
		    tmp[tmpx] = EOL;
		    m_output (tmp);
		    tmpx = 0;
		    part_ref (screen, (*screen).sc_up, N_LINES - 1);
		}
#endif /* SCO */
	    } else if (ch == 'L') {	/* IL Insert Line */
		int     j = args[0];

		do {
		    k = (*screen).sclines[(*screen).sc_lo];
		    for (i = (*screen).sc_lo; i > ypos[HOME]; i--)
			(*screen).sclines[i] = (*screen).sclines[i - 1];
		    (*screen).sclines[ypos[HOME]] = k;
		    for (i = 0; i < N_COLUMNS; i++) {
			(*screen).screenx[k][i] = SP;
			(*screen).screena[k][i] = (*screen).att;
#ifdef COLOR
			(*screen).screenc[k][i] = (*screen).col;
#endif /* COLOR */
		    }
		} while (--j > 0);
		xpos[HOME] = 0;
#ifdef SCO
		if (foreground) {
		    tmp[tmpx - 1] = CAN;	/* do not send IL */
		    tmp[tmpx] = EOL;
		    m_output (tmp);
		    tmpx = 0;
		    part_ref (screen, (*screen).sc_up, N_LINES - 1);
		}
#endif /* SCO */
	    } else if (ch == 'S') {	/* SU Scroll up   */
		int     j = args[0];

		do {
		    k = (*screen).sclines[(*screen).sc_up];
		    for (i = (*screen).sc_up; i < (*screen).sc_lo; i++)
			(*screen).sclines[i] = (*screen).sclines[i + 1];
		    (*screen).sclines[i] = k;
		    for (i = 0; i < N_COLUMNS; i++) {
			(*screen).screenx[k][i] = SP;
			(*screen).screena[k][i] = (*screen).att;
#ifdef COLOR
			(*screen).screenc[k][i] = (*screen).col;
#endif /* COLOR */
		    }
		} while (--j > 0);
#ifdef SCO
		if (foreground) {
		    tmp[tmpx - 1] = 'A';	/* do not send SU */
		    tmp[tmpx] = EOL;
		    m_output (tmp);
		    tmpx = 0;
		    part_ref (screen, (*screen).sc_up, N_LINES - 1);
		}
#endif /* SCO */
	    } else if (ch == 'T') {	/* SD Scroll down */
		int     j = args[0];

		do {
		    k = (*screen).sclines[(*screen).sc_lo];
		    for (i = (*screen).sc_lo; i > (*screen).sc_up; i--)
			(*screen).sclines[i] = (*screen).sclines[i - 1];
		    (*screen).sclines[i] = k;
		    for (i = 0; i < N_COLUMNS; i++) {
			(*screen).screenx[k][i] = SP;
			(*screen).screena[k][i] = (*screen).att;
#ifdef COLOR
			(*screen).screenc[k][i] = (*screen).col;
#endif /* COLOR */
		    }
		} while (--j > 0);
#ifdef SCO
		if (foreground) {
		    tmp[tmpx - 1] = 'A';	/* do not send SD */
		    tmp[tmpx] = EOL;
		    m_output (tmp);
		    tmpx = 0;
		    part_ref (screen, (*screen).sc_up, N_LINES - 1);
		}
#endif /* SCO */
	    } else if (ch == 'Z') {	/* CBT Cursor backward tab */
		if ((i = xpos[HOME] - 1) < 1)
		    i = 1;
		do {
		    while ((*screen).tabs[--i] == 0) {
			if (i < 0) {
			    i++;
			    break;
			}
		    }
		    if (i == 0)
			break;
		} while (args[0]-- > 0);
		xpos[HOME] = i;
#ifdef PROC_TAB
		tmp[tmpx++] = CR;
		if ((xpos[HOME] = i) != 0) {
		    tmp[tmpx++] = ESC;
		    tmp[tmpx++] = '[';
		    if (i > 9)
			tmp[tmpx++] = i / 10 + '0';
		    tmp[tmpx++] = i % 10 + '0';
		    tmp[tmpx++] = 'C';
		}
#endif /* PROC_TAB */
	    }
	    if (ch != '~')
		goto zasend;
/* ZAS Alternate Screen stuff */
/* the following is better programmed with     */
/* 'switch' but some compilers do not have     */
/* enough power to swallow the implied stack   */
/* depth                                       */
/* switch (args[0]) */
	    i = args[0];
	    if (i == 1)
		goto zas1;
	    if (i == 2)
		goto zas2;
	    if (i == 3)
		goto zas3;
	    if (i == 4)
		goto zas4;
	    if (i == 5)
		goto zas5;
	    if (i != 0)
		goto zas6;
/* zas0: exchange foreground/background */
	    tmp[tmpx] = EOL;
	    tmpx = 0;
	    if (foreground)
		m_output (tmp);
/* exchange parameters of screen */
	    (*screen).Xpos = xpos[HOME];
	    (*screen).Ypos = ypos[HOME];
	    vts = screen;
	    screen = altscr;
	    altscr = vts;
	    xpos[HOME] = (*screen).Xpos;
	    ypos[HOME] = (*screen).Ypos;

	  zas4:;			/* refresh foreground */
	    tmpx = 0;
	    if (foreground)
		part_ref (screen, 0, N_LINES - 1);
	    goto zasend;
	  zas1:;			/* foreground screen */
	    if (foreground)
		goto zasend;
	    foreground = TRUE;
/* exchange parameters of screen */
	  pPRIVATE:(*screen).Xpos = xpos[HOME];
	    (*screen).Ypos = ypos[HOME];
	    vts = screen;
	    screen = altscr;
	    altscr = vts;
	    xpos[HOME] = (*screen).Xpos;
	    ypos[HOME] = (*screen).Ypos;
	    tmpx = 0;
	    goto zasend;
	  zas2:;			/* background screen */
	    if (foreground == FALSE)
		goto zasend;
	    tmp[tmpx] = EOL;
	    m_output (tmp);
	    foreground = FALSE;
	    goto pPRIVATE;
	  zas3:;			/* save foreground to back */
	    stcpy0 (altscr, screen, sizeof (struct vtstyp));

	    goto zasend;
	  zas5:;			/* push screen */
	    if (zaslevel >= MAXZAS)
		goto zasend;
	    vts = (struct vtstyp *) calloc (1, sizeof (struct vtstyp));

	    zases[zaslevel++] = vts;
	    (*screen).Xpos = xpos[HOME];
	    (*screen).Ypos = ypos[HOME];
	    stcpy0 (vts, screen, sizeof (struct vtstyp));

	    goto zasend;
	  zas6:;			/* pop  screen */
	    if (--zaslevel < 0) {
		zaslevel = 0;
		goto zasend;
	    }
	    vts = zases[zaslevel];
#ifdef SCO
	    i = (*screen).bw;		/* do not pop bw state */
#endif /* SCO */
	    stcpy0 (screen, vts, sizeof (struct vtstyp));

#ifdef SCO
	    (*screen).bw = i;
#endif /* SCO */
	    xpos[HOME] = (*screen).Xpos;
	    ypos[HOME] = (*screen).Ypos;
	    free (vts);
	    goto zas4;
/* end zas6 */
	  zasend:;
	    argcnt = 0;
	    noargs = TRUE;
	    esc = 0;
	    continue;
	}				/* end 'printable' characters */
	if ((esc = (ch == ESC))) {
	    for (ch = 0; ch < ARGS_IN_ESC; args[ch++] = 0) ;
	    argcnt = 0;
	    noargs = TRUE;
	    continue;
	}
	if (ch == LF) {
	  pLF:;
	    if ((ypos[HOME] <= (*screen).sc_up) || (ypos[HOME] > (*screen).sc_lo)) {
		if (++ypos[HOME] >= (N_LINES - 1)) {
		    ypos[HOME] = (N_LINES - 1);
		}
		if (ch == LF)
		    tmpx--;
		tmp[tmpx++] = ESC;
		tmp[tmpx++] = '[';
		if (ypos[HOME] > 8)
		    tmp[tmpx++] = (1 + ypos[HOME]) / 10 + '0';
		tmp[tmpx++] = (1 + ypos[HOME]) % 10 + '0';
		if (xpos[HOME] > 0) {
		    tmp[tmpx++] = ';';
		    if (xpos[HOME] > 8)
			tmp[tmpx++] = (1 + xpos[HOME]) / 10 + '0';
		    tmp[tmpx++] = (1 + xpos[HOME]) % 10 + '0';
		}
		tmp[tmpx++] = 'H';
	    }
/* within scroll area */
	    else if (ypos[HOME] < (*screen).sc_lo) {
		if (++ypos[HOME] >= (N_LINES - 1)) {
		    ypos[HOME] = (N_LINES - 1);
		}
	    }
/* lower margin of scroll area: scroll up */
	    else {
		xpos[HOME] = 0;
#ifdef SCO
		if ((ch != LF)
		    && (!(*screen).lin24)
		    && (ypos[HOME] == (*screen).sc_lo)
		    && (ypos[HOME] == N_LINES - 1))
		    continue;

#endif /* SCO */
/*                        if ((*screen).rollflag==FALSE) continue; */
		k = (*screen).sclines[(*screen).sc_up];
		for (i = (*screen).sc_up; i < (*screen).sc_lo; i++)
		    (*screen).sclines[i] = (*screen).sclines[i + 1];
		(*screen).sclines[(*screen).sc_lo] = k;
		for (i = 0; i < N_COLUMNS; i++) {
		    (*screen).screenx[k][i] = SP;
		    (*screen).screena[k][i] = (*screen).att;
#ifdef COLOR
		    (*screen).screenc[k][i] = (*screen).col;
#endif /* COLOR */
		}
#ifdef SCO
		tmp[tmpx] = EOL;
		tmpx = 0;
		if (foreground) {
		    m_output (tmp);
		    part_ref (screen, (*screen).sc_up, (*screen).sc_lo);
		}
#endif /* SCO */
	    }
	    continue;
	}
	if (ch == CR) {
	    xpos[HOME] = 0;
	    continue;
	}
	if (ch == BS) {
	    if (--xpos[HOME] < 0)
		xpos[HOME] = 0;
	    continue;
	}
/* FORM FEED: clear screen */
	if (ch == FF) {
	    xpos[HOME] = 0;
	    ypos[HOME] = 0;
	    ch = 'J';
	    tmp[tmpx - 1] = ESC;
	    tmp[tmpx++] = '[';
	    tmp[tmpx++] = 'H';
	    tmp[tmpx++] = ESC;
	    tmp[tmpx++] = '[';
	    tmp[tmpx++] = ch;
	    args[0] = 0;
	    noargs = TRUE;
	    argcnt = 0;
	    goto ED;
	}
	if (ch == TAB) {
	    k = xpos[HOME];
	    if ((i = k + 1) >= N_COLUMNS)
		i = (N_COLUMNS - 1);
	    while ((*screen).tabs[i] == 0) {
		if (i >= N_COLUMNS) {
		    i = (N_COLUMNS - 1);
		    break;
		}
		i++;
	    }
#ifdef PROC_TAB
	    if (i != k) {
		tmp[tmpx++] = CR;
		if ((xpos[HOME] = i) != 0) {
		    tmp[tmpx++] = ESC;
		    tmp[tmpx++] = '[';
		    if (i > 9)
			tmp[tmpx++] = i / 10 + '0';
		    tmp[tmpx++] = i % 10 + '0';
		    tmp[tmpx++] = 'C';
		}
	    }
#endif /* PROC_TAB */
	    continue;
	}
	if (ch == SI)			/* SI Shift In */
	    (*screen).att &= ~01;	/* clear SO-Bit */
	if (ch == SO)			/* SO Shift Out */
	    (*screen).att |= 01;	/* set SO-Bit */

	if (ch == CAN) {		/* CANcel ESC_esquence */
	    esc = argcnt = noargs = 0;
	    continue;
	}
    }					/* end while */
    tmp[tmpx] = EOL;
    if (foreground)
	m_output (tmp);
    return;
}					/* end writeHOME() */
/******************************************************************************/
#ifdef SCO
int
scosgr (att, bwflag)
	short   att,			/* screen attributes byte */
	        bwflag;			/* black_on_white flag */
{
    att = att & ~044;			/* suppress SGR(3) and SGR(6) */
    if (att & 0200)
	return att & 0201;		/* no display */
    if (bwflag)
	att ^= 0100;			/* toggle inverse */
    if (att & 0100)
	return att & 0121;		/* inverse, evtl. incl. blink */
    if (att & 032)
	return att & 033;		/* other valid combinations */
    return att;
}					/* end scosgr() */
#endif /* SCO */
/******************************************************************************/
void
ris (scr)				/* init Screen params */
	struct vtstyp *scr;		/* all info from Screen */
{
    short   i,
            l;

    scr->att = 0;
#ifdef COLOR
    scr->col = 7;			/* default color is white on black */
#endif /* COLOR */
    scr->Xpos = 0;
    scr->Ypos = 0;
    for (i = 0; i < CSLEN; i++) {
	scr->csx[i] = 0;
	scr->csy[i] = 0;
    }
    scr->cs = 0;
    scr->sc_up = 0;
    scr->sc_lo = N_LINES - 1;
    for (i = 0; i <= N_LINES; i++) {
	scr->sclines[i] = i;
	for (l = 0; l < N_COLUMNS; l++) {
	    scr->screenx[i][l] = SP;
	    scr->screena[i][l] = 0;
#ifdef COLOR
	    scr->screenc[i][l] = 7;	/* default color is white on black */
#endif /* COLOR */
	}
    }
/* TABS */
    for (i = 0; i <= N_COLUMNS; i++)
	scr->tabs[i] = 0;
    for (i = 7; i <= N_COLUMNS; i += 8)
	scr->tabs[i] = 1;
    scr->rollflag = TRUE;		/* Roll or Page mode */
    scr->lin24 = TRUE;			/* 24 lines or 25 lines mode */
    scr->savarg = FALSE;
    for (i = 0; i < CSLEN; i++)
	scr->cssgr[i] = FALSE;		/* save SGR flag */
    return;
}					/* end ris() */
/******************************************************************************/
void
part_ref (scr, from, to)		/* refresh (foreground) screen partially */
	struct vtstyp *scr;		/* all info from screen */
	short   from,			/* first line to be refreshed 0..23 */
	        to;			/* last line to be refreshed */
{
    short   k,
            l,
            max;
    unsigned char exa,
            exc;
    short   i;
    unsigned char *linea,
           *linex,
           *linec,
            ch;
    unsigned char tmp[1300];

/* build up alternate screen */
/* reset SGR,TBM;white on black,go HOME; 25 Lines */
    stcpy (tmp, "\017\033[m\033[37;40m\033[r\033[H\033[0u\201");
    m_output (tmp);
#ifndef PROC_TAB
    k = 0;
    i = 0;				/* set TABs */
    tmp[k++] = ESC;
    tmp[k++] = '[';
    if (from > 8)
	tmp[k++] = '0' + (from + 1) / 10;
    tmp[k++] = '0' + (from + 1) % 10;
    tmp[k++] = 'H';
    for (l = 0; l < N_COLUMNS; l++) {
	tmp[k++] = SP;
	if ((*scr).tabs[l]) {
	    tmp[k++] = ESC;
	    tmp[k++] = 'H';
	    i = k;
	}
    }
    tmp[i++] = CR;
    tmp[i] = EOL;
    m_output (tmp);
#endif /* PROC_TAB */
    k = 0;
    for (i = from; i <= to; i++) {
	tmp[k++] = ESC;			/* CUP(i) */
	tmp[k++] = '[';
	if (i > 8)
	    tmp[k++] = (i + 1) / 10 + '0';
	if (i > 0)
	    tmp[k++] = (i + 1) % 10 + '0';
	tmp[k++] = 'H';
	linex = (*scr).screenx[(unsigned int) (*scr).sclines[i]];
	linea = (*scr).screena[(unsigned int) (*scr).sclines[i]];
#ifdef COLOR
	linec = (*scr).screenc[(unsigned int) (*scr).sclines[i]];
#endif /* COLOR */
	for (max = N_COLUMNS - 1; max > 0; max--) {
	    if (linex[max] != SP)
		break;
	    if (linea[max] != linea[max - 1])
		break;
	    if (linec[max] != linec[max - 1])
		break;
	}
	exa = ~linea[0];		/* dummy value to trigger argument codes on 1st char */
	exc = ~linec[0];		/* dummy value to trigger argument codes on 1st char */
	for (l = 0; l <= max; l++) {
	    if (l == (N_COLUMNS - 1) && (i == (N_LINES)))
		continue;
#ifndef LINUX
#ifdef COLOR
	    if (exc != linec[l]) {	/* set color */
		if ((exc / 16) != (linec[l] / 16)) {	/* background color */
		    tmp[k++] = ESC;
		    tmp[k++] = '[';
		    tmp[k++] = '=';
		    tmp[k++] = '0';
		    if (linec[l] / 16 > 9)
			tmp[k++] = '0' + linec[l] / 16 / 10;
		    tmp[k++] = '0' + linec[l] / 16 % 10;
		    tmp[k++] = 'G';
		    tmp[k++] = ESC;	/* background == reverse foreground */
		    tmp[k++] = '[';
		    tmp[k++] = '=';
		    tmp[k++] = '0';
		    if (linec[l] / 16 > 9)
			tmp[k++] = '0' + linec[l] / 16 / 10;
		    tmp[k++] = '0' + linec[l] / 16 % 10;
		    tmp[k++] = 'H';
		}
		if ((exc % 16) != (linec[l] % 16)) {	/* foreground color */
		    tmp[k++] = ESC;
		    tmp[k++] = '[';
		    tmp[k++] = '=';
		    tmp[k++] = '0';
		    if (linec[l] % 16 > 9)
			tmp[k++] = '0' + linec[l] % 16 / 10;
		    tmp[k++] = '0' + linec[l] % 16 % 10;
		    tmp[k++] = 'F';
		    tmp[k++] = ESC;	/* foreground == reverse background */
		    tmp[k++] = '[';
		    tmp[k++] = '=';
		    tmp[k++] = '0';
		    if (linec[l] % 16 > 9)
			tmp[k++] = '0' + linec[l] % 16 / 10;
		    tmp[k++] = '0' + linec[l] % 16 % 10;
		    tmp[k++] = 'I';
		}
		exc = linec[l];
	    }
#endif /* COLOR */
#endif /* LINUX */
	    if (exa != linea[l]) {	/* set attribute */
		short   p,
		        xatt;

		p = exa;
		exa = linea[l];
		if ((p & 01) != (exa & 01))
		    tmp[k++] = (exa & 01) ? SO : SI;
		if ((p & ~01) != (exa & ~01)) {
		    xatt = exa;
#ifdef SCO
		    xatt = scosgr (xatt, (*scr).bw);
#endif /* SCO */
		    tmp[k++] = ESC;
		    tmp[k++] = '[';
#ifdef SCO
		    if ((*scr).bw)
			tmp[k++] = '7';
#endif /* SCO */
		    tmp[k++] = 'm';
		    tmp[k++] = ESC;
		    tmp[k++] = '[';
		    if (xatt & 02) {
#ifdef SCO
			tmp[k++] = '1';
#else
			tmp[k++] = '2';
#endif /* SCO */
			tmp[k++] = ';';
		    }
		    if (xatt & 04) {
			tmp[k++] = '3';
			tmp[k++] = ';';
		    }
		    if (xatt & 010) {
			tmp[k++] = '4';
			tmp[k++] = ';';
		    }
		    if (xatt & 020) {
			tmp[k++] = '5';
			tmp[k++] = ';';
		    }
		    if (xatt & 040) {
			tmp[k++] = '6';
			tmp[k++] = ';';
		    }
		    if (xatt & 0100) {
			tmp[k++] = '7';
			tmp[k++] = ';';
		    }
		    if (xatt & 0200) {
			tmp[k++] = '8';
			tmp[k++] = ';';
		    }
		    if (tmp[k - 1] == ';')
			k--;
		    tmp[k++] = 'm';
		}
	    }
	    tmp[k++] = linex[l];
	}
	if (max + 1 < N_COLUMNS) {
	    tmp[k++] = ESC;
	    tmp[k++] = '[';
	    tmp[k++] = 'K';
	}
	tmp[k] = EOL;
	k = mcmnd;
	mcmnd = 'w';
	ch = (*codptr);
	*codptr = ',';
	m_output (tmp);
	*codptr = ch;
	mcmnd = k;
	k = 0;
    }
/* 'saved' cursor position */
/* actual cursor position & actual attribute */
/* 'flush' output */
    k = 0;
#ifndef SCO
    tmp[k++] = ESC;			/* restore CursorSave Position */
    tmp[k++] = '[';
    i = (*scr).csy[(*scr).cs] + 1;
    if (i > 9)
	tmp[k++] = '0' + i / 10;
    tmp[k++] = '0' + i % 10;
    tmp[k++] = ';';
    i = (*scr).csx[(*scr).cs] + 1;
    if (i > 9)
	tmp[k++] = '0' + i / 10;
    tmp[k++] = '0' + i % 10;
    tmp[k++] = 'H';
    tmp[k++] = ESC;			/* DEC Cursor SAVE */
    tmp[k++] = '7';
    tmp[k++] = ESC;			/* ANSI (?) Cursor SAVE */
    tmp[k++] = '[';
    tmp[k++] = 's';
    tmp[k++] = ESC;			/* restore 24/25 line mode */
    tmp[k++] = '[';
    tmp[k++] = '0' + (*scr).lin24;
    tmp[k++] = 'u';
    tmp[k++] = ESC;			/* restore Scroll area */
    tmp[k++] = '[';
    if (((*scr).sc_up) > 8)
	tmp[k++] = '0' + ((*scr).sc_up + 1) / 10;
    tmp[k++] = '0' + ((*scr).sc_up + 1) % 10;
    tmp[k++] = ';';
    if (((*scr).sc_lo) > 8)
	tmp[k++] = '0' + ((*scr).sc_lo + 1) / 10;
    tmp[k++] = '0' + ((*scr).sc_lo + 1) % 10;
    tmp[k++] = 'r';
#endif /* not SCO */
#ifndef LINUX
#ifdef COLOR
    tmp[k++] = ESC;			/* restore foreground color */
    tmp[k++] = '[';
    tmp[k++] = '=';
    if (((*scr).col) % 16 > 9)
	tmp[k++] = '0' + ((*scr).col) % 16 / 10;
    tmp[k++] = '0' + ((*scr).col) % 16 % 10;
    tmp[k++] = 'F';
    tmp[k++] = ESC;			/* restore reverse background color */
    tmp[k++] = '[';
    tmp[k++] = '=';
    if (((*scr).col) % 16 > 9)
	tmp[k++] = '0' + ((*scr).col) % 16 / 10;
    tmp[k++] = '0' + ((*scr).col) % 16 % 10;
    tmp[k++] = 'I';
    tmp[k++] = ESC;			/* restore background color */
    tmp[k++] = '[';
    tmp[k++] = '=';
    if (((*scr).col) / 16 > 9)
	tmp[k++] = '0' + ((*scr).col) / 16 / 10;
    tmp[k++] = '0' + ((*scr).col) / 16 % 10;
    tmp[k++] = 'G';
    tmp[k++] = ESC;			/* restore reverse foreground color */
    tmp[k++] = '[';
    tmp[k++] = '=';
    if (((*scr).col) / 16 > 9)
	tmp[k++] = '0' + ((*scr).col) / 16 / 10;
    tmp[k++] = '0' + ((*scr).col) / 16 % 10;
    tmp[k++] = 'H';
#endif /* COLOR */
#endif /* LINUX */
    tmp[k++] = ESC;			/* restore CursorPosition */
    tmp[k++] = '[';
    if (ypos[HOME] > 8)
	tmp[k++] = '0' + (ypos[HOME] + 1) / 10;
    tmp[k++] = '0' + (ypos[HOME] + 1) % 10;
    tmp[k++] = ';';
    if (xpos[HOME] > 8)
	tmp[k++] = '0' + (xpos[HOME] + 1) / 10;
    tmp[k++] = '0' + (xpos[HOME] + 1) % 10;
    tmp[k++] = 'H';
    i = UNSIGN ((*scr).att);
    tmp[k++] = ((i & 01) ? SO : SI);
    tmp[k++] = ESC;			/* restore graphic attributes */
    tmp[k++] = '[';
#ifdef SCO
    if ((*scr).bw)
	tmp[k++] = '7';
    tmp[k++] = 'm';
    tmp[k++] = ESC;
    tmp[k++] = '[';
    i = scosgr (i, (*scr).bw);
    if (i & 02) {
	tmp[k++] = '1';
	tmp[k++] = ';';
    }
#else
    if (i & 02) {
	tmp[k++] = '2';
	tmp[k++] = ';';
    }
#endif /* SCO */
    if (i & 04) {
	tmp[k++] = '3';
	tmp[k++] = ';';
    }
    if (i & 010) {
	tmp[k++] = '4';
	tmp[k++] = ';';
    }
    if (i & 020) {
	tmp[k++] = '5';
	tmp[k++] = ';';
    }
    if (i & 040) {
	tmp[k++] = '6';
	tmp[k++] = ';';
    }
    if (i & 0100) {
	tmp[k++] = '7';
	tmp[k++] = ';';
    }
    if (i & 0200) {
	tmp[k++] = '8';
	tmp[k++] = ';';
    }
    if (tmp[k - 1] == ';')
	k--;
    tmp[k++] = 'm';
    tmp[k] = EOL;
    k = mcmnd;
    mcmnd = 0;
    m_output (tmp);
    mcmnd = k;

    return;
}					/* end part_ref() */
/******************************************************************************/
void
m_output (text)				/* delayed write for higher system throughput */
	char   *text;			/* write this string */

{
    static char buffer[2048];
    static int p = 0;
    char   *G;
    int     i,
            ch;

    if (io == HOME)
	opnfile[io] = stdout;
    G = SIflag[io] ? G0O[io] : G1O[io];
    i = 0;
    while ((ch = text[i++]) != EOL) {
	ch = UNSIGN (ch);
	if (ESCflag[io] == FALSE
	    && ch >= NUL && ch < 255)
	    ch = G[ch];
	buffer[p++] = ch;
	if (ch == SI) {
	    SIflag[io] = TRUE;
	    G = G0O[io];
	}
	if (ch == SO) {
	    SIflag[io] = FALSE;
	    G = G1O[io];
	}
	if (ch == ESC) {
	    ESCflag[io] = TRUE;
	    continue;
	}
	if (ch == '[' && ESCflag[io]) {
	    ESCflag[io] = 2;
	    continue;
	}
	if (ch >= '@' && ch < DEL)
	    ESCflag[io] = FALSE;
	if (ch != NUL)
	    continue;
	fputs (buffer, opnfile[io]);
	p = 0;				/* NUL needs special treatment */
	fputc (NUL, opnfile[io]);	/* 'cause unix uses it as delim */
    }
    buffer[p] = NUL;
    if (mcmnd == 'w' && *codptr == ',' && (p < 1536))
	return;
    if (nodelay[io]) {
	if (setjmp (sjbuf)) {
	    ierr = NOWRITE;
	    goto done;
	}
	SIGNAL_ACTION(SIGALRM, ontimo, NULL);
	alarm (3);
    }
    fputs (buffer, opnfile[io]);
    p = 0;
    if (mcmnd == '*')
	fputc (EOL, opnfile[io]);	/* special treatment for EOL */
    if (demomode)
	fputc (d0char, opnfile[io]);
    fflush (opnfile[io]);
  done:alarm (0);			/* reset alarm request */
    return;
}					/* end of m_output() */
/******************************************************************************/
void
write_m (text)
	char   *text;			/* write this string */

{
    static char esc = 0;		/* esc processing flag */
    static char tmp[512];
    register i,
            j,
            ch;

    if (io == HOME) {
	opnfile[HOME] = stdout;
	if (filter == FALSE) {
	    writeHOME (text);
	    return;
	}
    }
/* 'normal' output for devices other than HOME */
    j = 0;
    i = 0;
    while ((ch = text[j++]) != EOL) {
	tmp[i++] = ch;
	if (ch >= SP) {
	    if (esc == 0) {
		xpos[io]++;
		continue;
	    }
	    if (ch == '[') {
		esc = 2;
		continue;
	    }
	    if (ch >= '@' || esc == 1)
		esc = 0;
	    continue;
	}
	esc = (ch == ESC);
	if (ch == LF) {
	    ypos[io]++;
	    if (crlf[io])
		xpos[io] = 0;
	    continue;
	}
	if (ch == CR) {
	    xpos[io] = 0;
	    continue;
	}
	if (ch == BS) {
	    if (--xpos[io] < 0)
		xpos[io] = 0;
	    continue;
	}
	if (ch == FF) {
	    xpos[io] = 0;
	    ypos[io] = 0;
	    continue;
	}
	if (ch == TAB) {
	    xpos[io] += 8 - (xpos[io] % 8);
	}
    }					/* end while */
    tmp[i] = EOL;
    m_output (tmp);
    xpos[io] &= 0377;
    ypos[io] &= 0377;
    return;
}					/* end of write_m() */
/******************************************************************************/
void
write_t (col)
	short int col;			/* tab to number 'col' column' */
{
    short int i,
            j;
    char    tmp[256];			/* Not tied to STRLEN necessarily*/

    if (col <= xpos[io])
	return;
    if (col > (sizeof(tmp)-1))
	col = (sizeof(tmp)-1);		/* guard against buffer overflow */
    j = xpos[io];
    i = 0;
    while (j++ < col)
	tmp[i++] = SP;
    tmp[i] = EOL;
    write_m (tmp);
    return;
}					/* end of write_t() */
/******************************************************************************/
void
ontimo ()
{					/* handle timeout (for read) */
    longjmp (sjbuf, 1);
}					/* end of ontimo() */

/******************************************************************************/
void
read_m (stuff, timeout, timeoutms, length)
	char   *stuff;
	long    timeout;
	short   timeoutms;
	short   length;

/*  Try to read up to 'length' characters from current 'io'
 * If 'length' == zero a single character read is performed and
 * its $ASCII value is returned in 'stuff'.
 * A negative value of 'timeout' means there's no timeout.
 * a zero value looks whether a character has been typed in already
 * a positive value terminates the read after 'timeout' seconds
 * a timeout will set $TEST to TRUE if successfull, FALSE otherwise
 * 
 * timeouts 1 .. 3 are done by polling,
 * higher timeouts request an alarm. that different treatment is
 * necessary, because timeouts may be up to 2 seconds early.
 * polling on higher timeouts would be require too much system resources
 * just for nothing.
 * maximum timeout is 32767 (2**15-1) seconds (about 9 hrs)
 * There are no provisions to handle overflow.
 */
{
    static char previous[2][256] =
    {
	{EOL},
	{EOL}};				/* all static: longjmp! */
    static short p_toggle = 0;
    static char term_key[256] =
    {EOL, EOL};
    static short int escptr;
    static short int single;
    static short int timoflag;		/* timeout flag */
    static long int timex;
    static short int timexms;
    static short int i;
    static int ch;
#ifdef USE_GETTIMEOFDAY
    static struct timeval timebuffer;
#else
    static struct timeb timebuffer;
#endif

    escptr = 0;
    single = 0;
    timoflag = FALSE;
    if (length > 255)			/* Not necessarily tied to STRLEN*/
	length = 255;
    stuff[length] = EOL;
    stuff[0] = EOL;
    if (length < 1)
	single = length = 1;
    timex = 0L;
    timexms = 0;

    if (io == HOME)
	opnfile[HOME] = stdin;
    if (io == HOME && !filter) {
	if (NOTYPEAHEAD)
	    fflush (stdin);		/* ignore previous input */
	if (timeout >= 0) {
	    test = TRUE;
	    if (timeout > 2 && timeout <= 32767) {	/* cave small/large values */
		if (setjmp (sjbuf)) {
		    test = FALSE;
		    timoflag = TRUE;
		    goto done;
		}
		SIGNAL_ACTION(SIGALRM, ontimo, NULL);
		alarm ((unsigned) timeout);
		timeout = (-1L);
	    } else {
		if (timeout >= 0) {
#ifdef USE_GETTIMEOFDAY
		    gettimeofday (&timebuffer, NULL);	/* get current time */
		    timeout += timebuffer.tv_sec;	/* target time */
		    timeoutms += timebuffer.tv_usec;
#else
		    ftime (&timebuffer);	/* get current time */
		    timeout += timebuffer.time;		/* target time */
		    timeoutms += timebuffer.millitm;
#endif
		    if (timeoutms > 999) {
			timeoutms -= 1000;
			timeout++;
		    }
		}
	    }
	}
	zb[0] = EOL;
	zb[1] = EOL;
	zcc = FALSE;
	for (i = 0; i < length; i++) {
	    if (ierr == INRPT) {
		if (--i < 0)
		    i = 0;
		stuff[i] = EOL;
		break;
	    }
	    if (jour_flag < 0) {
		if ((ch = fgetc (jouraccess)) == EOF) {
		    jour_flag = 0;
		    fclose (jouraccess);
		    jouraccess = NULL;
		} else {
		    if (ch == SI) {
			test = FALSE;
			break;
		    }			/* timeout char */
		}
	    }
	    if (jour_flag >= 0) {
		if (ug_buf[HOME][0] != EOL) {
		    ch = ug_buf[HOME][0];
		    stcpy (ug_buf[HOME], &ug_buf[HOME][1]);
		} else {
		    if (timeout >= 0)
#ifdef SYSFIVE
		    {
			if (rdchk0 (&ch) == 1) {
			}
#else
		    {
			if (rdchk (fileno (stdin)) > 0)
			    ch = getc (stdin);
#endif /* SYSFIVE */
			else {
			    if (timeout < 0) {
				test = FALSE;
				timoflag = TRUE;
				stuff[i] = EOL;
				break;
			    }
#ifdef USE_GETTIMEOFDAY
			    gettimeofday (&timebuffer, NULL);	/* get current time */
			    if ((timeout < timebuffer.tv_sec) ||
				((timeout == timebuffer.tv_sec) &&
				 (timeoutms <= timebuffer.tv_usec))) {
#else
			    ftime (&timebuffer);	/* get current time */
			    if ((timeout < timebuffer.time) ||
				((timeout == timebuffer.time) &&
				 (timeoutms <= timebuffer.millitm))) {
#endif
				test = FALSE;
				timoflag = TRUE;
				stuff[i] = EOL;
				break;
			    }
			    i--;
			    continue;
			}
		    } else
			ch = getc (stdin);
		}
	    }
	    if (UNSIGN (ch) == 255) {
		i--;
		continue;
	    }				/* illegal char */
	    if (CONVUPPER) {		/* convert to uppercase ? */
		if (ch >= 'a' && ch <= 'z')
		    ch -= 32;
	    }
	    if (ch == SI		/* on CTRL/O suspend execution until next CTRL/O */
		&& CTRLOPROC) {
		printf ("\033[8p\033[4u");	/* screen dark, timeout off */
		while ((ch = getc (stdin)) != SI) ;	/* loop until CTRL/O */
		printf ("\033[9p\033[5u");	/* screen light, timeout on */
		i--;
		continue;
	    }
	    zb[0] = ch;
	    if (single == 0) {
		if (ch == ESC) {
		    i--;
		    term_key[0] = ESC;
		    escptr = 1;
		    continue;
		}
		if (escptr) {
		    short   j;

		    i--;
		    term_key[escptr++] = ch;
		    term_key[escptr] = EOL;
		    if (ch == '[' && escptr == 2)
			continue;	/* CSI */
/* DECs PF1..PF4 and Keypad application */
		    if (ch == 'O' && PF1flag && escptr == 2)
			continue;
		    if (escptr > 2 && (ch < '@' || ch >= DEL))
			continue;
		    if (ESCSEQPROC) {
			stcpy (zb, term_key);
			break;
		    }
		    if (escptr == 2) {
			if (ch == '9') {	/* to begin of string */
			    while (i >= 0) {
				if (ECHOON) {
				    if (xpos[HOME] > 0)
					write_m ("\033[D\201");
				    else
					write_m ("\033M\033[79C\201");
				}
				i--;
			    }
			    escptr = 0;
			    continue;
			} else if (ch == ':') {		/* to end of string */
			    while (stuff[i] != EOL && stuff[i + 1] != EOL) {
				if (ECHOON) {
				    if (xpos[HOME] < (N_COLUMNS - 1))
					write_m ("\033[C\201");
				    else
					write_m ("\012\015\201");
				}
				i++;
			    }
			    escptr = 0;
			    continue;
			}
		    }
		    if (escptr == 3) {
			if (ch == 'D') {	/* CUB left arrow */
			    if (i >= 0) {
				if (ECHOON) {
				    if (xpos[HOME] > 0)
					write_m (term_key);
				    else
					write_m ("\033M\033[79C\201");
				}
				i--;
			    }
			    escptr = 0;
			    continue;
			}
			if (stuff[i + 1] == EOL) {
			    escptr = 0;
			    continue;
			}
			if (ch == 'C') {	/* CUF right arrow */
			    if (ECHOON) {
				if (xpos[HOME] < (N_COLUMNS - 1))
				    write_m (term_key);
				else
				    write_m ("\012\015\201");
			    }
			    i++;
			    escptr = 0;
			    continue;
			}
			if (ch == 'P') {	/* DCH Delete Character */
			    ch = i + 1;
			    if (stuff[ch] != EOL)
				while ((stuff[ch] = stuff[ch + 1]) != EOL)
				    ch++;
			    if (ECHOON) {
				ch = xpos[HOME] + 1;
				j = ypos[HOME] + 1;
				stcpy (term_key, &stuff[i + 1]);
				stcat (term_key, " \033[\201");
				intstr (&term_key[stlen (term_key)], j);
				stcat (term_key, ";\201");
				intstr (&term_key[stlen (term_key)], ch);
				stcat (term_key, "H\201");
				write_m (term_key);
			    }
			    escptr = 0;
			    continue;
			}
			if (ch == '@') {	/* ICH Insert Character */
			    ch = i;
			    while (stuff[ch++] != EOL) ;
			    while (ch > i) {
				stuff[ch] = stuff[ch - 1];
				ch--;
			    }
			    stuff[i + 1] = SP;
			    if (ECHOON) {
				ch = xpos[HOME] + 1;
				j = ypos[HOME] + 1;
				stcpy (term_key, &stuff[i + 1]);
				stcat (term_key, "\033[\201");
				intstr (&term_key[stlen (term_key)], j);
				stcat (term_key, ";\201");
				intstr (&term_key[stlen (term_key)], ch);
				stcat (term_key, "H\201");
				write_m (term_key);
			    }
			    escptr = 0;
			    continue;
			}
		    }
/* VT100 Functionkey */
		    if (ch == '~' && (escptr == 4 || escptr == 5)) {
			j = term_key[2] - '0';
			if (term_key[3] != ch)
			    j = j * 10 + term_key[3] - '0';
			if (j < 1 || j > 44)
			    j = 0;
		    }
/* SCO Functionkey */
		    else
			j = find ("@ABCDFGHIJKLMNOP0_dTVX ;\"#$%&\'<=*+,-./123UWY\201", &term_key[1]);
		    escptr = 0;
		    if (j == 0 || zfunkey[j - 1][0] == EOL)	/* key unknown or without a value */
			continue;
/* put key in input buffer, ignore overflow */
		    {
			char    tmp[256];

			stcpy (tmp, ug_buf[HOME]);
			stcpy (ug_buf[HOME], zfunkey[--j]);
			stcat (ug_buf[HOME], tmp);
		    }
		    continue;
		}
		term_key[0] = ch;
		term_key[1] = EOL;
		if (find (LineTerm, term_key))
		    break;
		if (ch == BS || ch == DEL) {
		    if (stuff[i] != EOL) {	/* DEL within string */
			if (ch == BS && i > 0) {
			    if (ECHOON) {
				if (xpos[HOME] > 0)
				    write_m ("\033[D\201");
				else
				    write_m ("\033M\033[79C\201");
			    }
			    i--;
			}
			ch = i;
			if (stuff[ch] != EOL)
			    while ((stuff[ch] = stuff[ch + 1]) != EOL)
				ch++;
			if (ECHOON) {
			    int     j;

			    ch = xpos[HOME] + 1;
			    j = ypos[HOME] + 1;
			    stcpy (term_key, &stuff[i]);
			    stcat (term_key, " \033[\201");
			    intstr (&term_key[stlen (term_key)], j);
			    stcat (term_key, ";\201");
			    intstr (&term_key[stlen (term_key)], ch);
			    stcat (term_key, "H\201");
			    write_m (term_key);
			}
			continue;
		    }
		    if (i > 0) {
			if (ECHOON) {
			    if (DELMODE) {
				if (xpos[HOME] > 0)
				    write_m ("\010 \010\201");
				else
				    write_m ("\033M\033[79C\033[P\201");
			    } else
				write_m ("\\\201");
			}
			i--;
			ch = i;
			while ((stuff[ch] = stuff[ch + 1]) != EOL)
			    ch++;
		    }
/* empty string delete? */
		    else if (DELEMPTY) {
			stuff[0] = EOL;
			i = 1;
			term_key[0] = ch;
			break;
		    }
		    i--;
		    continue;
		}
		if (ch == NAK || ch == DC2) {	/* CTRL/U deletes all input */
		    while (stuff[i] != EOL) {
			if (ECHOON) {
			    if (xpos[HOME] < (N_COLUMNS - 1))
				write_m ("\033[C\201");
			    else
				write_m ("\012\015\201");
			}
			i++;
		    }
		    while (i > 0) {
			if (ECHOON) {
			    if (DELMODE) {
				if (xpos[HOME] > 0)
				    write_m ("\010 \010\201");
				else
				    write_m ("\033M\033[79C\033[P\201");
			    } else
				write_m ("\\\201");
			}
			stuff[i--] = EOL;
		    }
		    stuff[i--] = EOL;
		    if (ch == NAK)
			continue;
/* (ch==DC2) *//* CTRL/R Retypes last input */
		    i = stcpy (stuff, previous[p_toggle]);
		    stuff[i--] = EOL;
		    toggle (p_toggle);
		    if (ECHOON)
			write_m (stuff);
		    continue;
		}
		if (ch == STX && (zbreakon || hardcopy)) {
		    if (zbreakon)
			zbflag = TRUE;
		    if (i > 0) {
			if (ECHOON)
			    write_m ("\010 \010\201");
		    }
		    i--;
		    continue;
		}
		if (ch == BrkKey) {	/* CTRL/C may interrupt */
		    i--;
		    if (breakon) {
			stuff[i] = EOL;
			ierr = INRPT;
			return;
		    }
		    continue;
		}
/* ignore non programmed CTRL/ key ?? */
		if (ch < SP &&
#ifdef NEWSTACK
		    (ch != TAB || stack) &&	/* TAB may be a programmed key */
#else
		    (ch != TAB || nstx) &&	/* TAB may be a programmed key */
#endif
		    NOCTRLS) {
		    i--;
		    continue;
		}
	    }
	    if (stuff[i] == EOL)
		stuff[i + 1] = EOL;
	    if (ch < 255)
		ch = (SIflag[io] ? G0I[io] : G1I[io])[UNSIGN (ch)];
	    stuff[i] = ch;
	    if (ECHOON) {
		term_key[0] = ch;
		term_key[1] = EOL;
		write_m (term_key);
	    }
	}
    } else {				/* $io != HOME */
	int     fd,
	        fdstat;

	fd = 0;
#ifdef SYSFIVE
	if (timeout >= 0) {
	    fd = fileno (opnfile[io]);
	    fdstat = fcntl (fd, F_GETFL);
	    fcntl (fd, F_SETFL, fdstat | O_NDELAY);
	}
#endif /* SYSFIVE */
	for (i = 0; i < length; i++) {
	    if (ierr == INRPT) {
		if (--i < 0)
		    i = 0;
		stuff[i] = EOL;
		break;
	    }
	    if (ug_buf[io][0] != EOL) {
		ch = ug_buf[io][0];
		stcpy (ug_buf[io], &ug_buf[io][1]);
	    } else if (timeout >= 0) {
		test = TRUE;
		if (
#ifdef SYSFIVE
		       fread (&ch, 1, 1, opnfile[io]) < 1)
#else
		       rdchk (fileno (opnfile[io])) == 0 ||
		       ((ch = fgetc (opnfile[io])) == EOF))
#endif /* SYSFIVE */
		{
		    if (--timeout < 0) {
			test = FALSE;
			timoflag = TRUE;
			stuff[i] = EOL;
			break;
		    }
		    sleep (1);
		    i--;
		    continue;
		}
		ch = UNSIGN (ch);
	    } else {
		ch = fgetc (opnfile[io]);
	    }
	    stuff[i] = ch;
	    if (ch == EOF) {
		if ((io == HOME) && filter)	/* EOF halts filtered mps */
		    ierr = INRPT;
		stuff[i] = EOL;
		break;
	    }
	    if (single)
		break;
	    if (ch == LF && crlf[io]) {
		i--;
		continue;
	    }
	    if ((ch == CR) || (ch == LF)) {
		int     ch0;

		stuff[i] = EOL;
/* if input terminates with CR/LF or LF/CR take both    */
/* as a single termination character. So it is possible */
/* to get correct input from files written by mumps     */
/* itself with CR/LF line termination                   */

#ifdef SYSFIVE
		if (fd == 0) {
		    fd = fileno (opnfile[io]);
		    fdstat = fcntl (fd, F_GETFL);
		    fcntl (fd, F_SETFL, fdstat | O_NDELAY);
		}
		if (fread (&ch0, 1, 1, opnfile[io]) == 1) {
		    ch0 = UNSIGN (ch0);
#else
		if (rdchk (fileno (opnfile[io])) == 1) {
		    ch0 = fgetc (opnfile[io]);
#endif /* SYSFIVE */
		    if ((ch == CR && ch0 != LF) ||
			(ch == LF && ch0 != CR))
			ungetc (ch0, opnfile[io]);
		}
		break;
	    }
	    if (UNSIGN (stuff[i]) < 255)
		stuff[i] = (SIflag[io] ? G0I[io] : G1I[io])[UNSIGN (stuff[i])];

	}
#ifdef SYSFIVE
	if (fd)
	    fcntl (fd, F_SETFL, fdstat);
#endif /* SYSFIVE */
    }
  done:alarm (0);			/* reset alarm request */
    if (io == HOME && jour_flag > 0) {
	char    tmp[256];

	tmp[stcpy (tmp, stuff)] = NUL;
	fputs (tmp, jouraccess);
	if (timoflag) {
	    tmp[0] = SI;
	    tmp[1] = NUL;		/* CTRL/O as timeout char */
	    fputs (tmp, jouraccess);
	} else if ((i < length) ||	/*save termination char if meaningful */
		   (stuff[0] == NUL && zb[0] == ESC)) {
	    tmp[stcpy (tmp, zb)] = NUL;
	    fputs (tmp, jouraccess);
	}
    }
    if (single) {			/* to ASCII */
	if (timoflag) {
	    stuff[0] = '-';
	    stuff[1] = '1';
	    stuff[2] = EOL;
	} else {
	    intstr (stuff, UNSIGN (stuff[0]));
	}
    } else if (io == HOME && stuff[0] != EOL) {
	stcpy (previous[toggle (p_toggle)], stuff);
    }
    return;


}					/* end of read_m() */
/******************************************************************************/
void
hardcpf ()
{					/* output of hardcopy information */
    int     i;
    char    tmp[2],
            line[2000];
    FILE   *hcpdes;

					/* ignore signal while we are here */
    SIGNAL_ACTION(SIGQUIT, SIG_IGN, NULL);

/* OPEN hardcopyfile in 'append' mode for output */

    tmp[0] = 'a';
    tmp[1] = NUL;			/* NUL not EOL !!! */
/* always: a=WRITE append */
    if ((hcpdes = fopen (hcpyfile, tmp)) == NULL)
	goto hcend;

    {
	short   exa,
	        k,
	        l;

	exa = 0;
	k = 0;
	line[k++] = FF;
	line[k++] = LF;
	for (i = 0; i < N_LINES; i++) {
	    for (l = 0; l < N_COLUMNS; l++) {
		if (exa != (*screen).screena[(unsigned int) (*screen).sclines[i]][l]) {		/* set attribute */
		    short   p;

		    p = exa;
		    exa = (*screen).screena[(unsigned int) (*screen).sclines[i]][l];
		    if ((p & 01) != (exa & 01))
			line[k++] = (exa & 01) ? SO : SI;
		    if ((p & ~01) != (exa & ~01)) {
			line[k++] = ESC;
			line[k++] = '[';
			for (p = 1; p < 8; p++) {
			    if (exa & (1 << p)) {
#ifdef SCO
				if (p == 1) {
				    line[k++] = '1';
				    line[k++] = ';';
				    continue;
				}
#endif /* SCO */
				line[k++] = '1' + p;
				line[k++] = ';';
			    }
			}
			if (line[k - 1] == ';')
			    k--;
			line[k++] = 'm';
		    }
		}
		line[k++] = (*screen).screenx[(unsigned int) (*screen).sclines[i]][l];
	    }
	    line[k++] = LF;
	    line[k] = NUL;
	    fputs (line, hcpdes);
	    k = 0;
	}
    }
    fclose (hcpdes);

  hcend:;

    SIGNAL_ACTION(SIGQUIT, hardcpf, NULL);	/* restore handler */

    return;
}					/*end of hardcpf() */
/******************************************************************************/
#ifdef SYSFIVE
/* UNIX system V has no 'rdchk' function. We must do it ourselves */
int
rdchk0 (data)
	int    *data;
{
    static int x;
    static int firsttime = TRUE;
    int     retcode;
    char    ch;

    if (firsttime) {
	x = fcntl (0, F_GETFL);
	firsttime = FALSE;
    }
    fcntl (0, F_SETFL, x | O_NDELAY);
    retcode = read (0, &ch, 1);
    fcntl (0, F_SETFL, x);
    *data = (retcode > 0) ? ch : NUL;
    return retcode;
}					/* end rdchk0() */
/******************************************************************************/
/* under XENIX the system has an intrinsic function 'locking'.                */
/* under UNIX System V it must be done with fcntl.                            */
/* the difference is, that the XENIX locking blocks all other accesses        */
/* whereas UNIX System V gives only protection if all users of a file         */
/* use 'locking'.                                                             */
/* since 'fcntl' is not properly documented, the outcome in any cases of      */
/* errors is uncertain.                                                       */

int
locking (fd, action, count)
	int     fd;			/* file descriptor of file to be locked */
	int     action;			/* action to be performed */
	long    count;			/* area to be locked */
{
    struct flock lock;

    lock.l_whence = 1;			/* lock from current position */
    lock.l_start = 0;
    lock.l_len = count;
    lock.l_pid = getpid ();

    if (lonelyflag)
	return 0;			/* no LOCK if single user */
    switch (action) {
    case 0:				/* LK_UNLK free previously locked area */
	lock.l_type = F_UNLCK;
	fcntl (fd, F_SETLK, &lock);
	break;
    case 1:				/* LK_LOCK lock against others reading and writing my data */
	lock.l_type = F_WRLCK;
	fcntl (fd, F_SETLKW, &lock);
	break;
    case 2:				/* LK_NBLCK lock against others reading and writing my data, don't block */
	lock.l_type = F_WRLCK;
	fcntl (fd, F_SETLK, &lock);
	break;
    case 3:				/* LK_RLCK lock against others writing my data */
	lock.l_type = F_RDLCK;
	fcntl (fd, F_SETLKW, &lock);
	break;
    case 4:				/* LK_NBRLCK lock against others writing my data, don't block */
	lock.l_type = F_RDLCK;
	lock.l_pid = getpid ();
	fcntl (fd, F_SETLK, &lock);
    }
    return 0;				/* action properly performed */
}					/* end locking() */
/******************************************************************************/
#ifdef OLDUNIX
void
set_io (action)				/* set io_parameters to mumps/unix conventions */
	short   action;			/* 0 = UNIX; 1 = MUMPS */
{
    static char del;
    static char fs;
    struct termio tpara;

    if (filter)
	return;				/* nothing special if used as
					 * filter */
    fflush (stdin);
    fflush (stdout);
    fflush (stderr);
    ioctl (0, TCGETA, &tpara);		/* get paramters            */
    if (action == UNIX) {
	tpara.c_lflag |= (ECHO | ICANON);	/* enable echo/no cbreak mode */
	tpara.c_iflag |= ICRNL;		/* cr-lf mapping */
	tpara.c_oflag |= ONLCR;		/* cr-lf mapping */
	tpara.c_cc[VINTR] = del;	/* interrupt                */
	tpara.c_cc[VQUIT] = fs;		/* quit                     */
/* we desperately tried to do it with DEL - but it didn't work */
	tpara.c_cc[VMIN] = EOT;
	tpara.c_cc[VTIME] = -1;
    } else {				/* action == MUMPS */
	tpara.c_lflag &= (~(ECHO | ICANON));	/* disable echo/cbreak mode */
	tpara.c_iflag &= ~ICRNL;	/* cr-lf mapping */
	tpara.c_oflag &= ~ONLCR;	/* cr-lf mapping */
	del = tpara.c_cc[VINTR];	/* save previous state of   */
	fs = tpara.c_cc[VQUIT];		/* quit and interrupt       */
	tpara.c_cc[VINTR] = BrkKey;	/* interrupt = CTRL/C       */
	tpara.c_cc[VQUIT] = (zbreakon || hardcopy) ? STX : -1;	/* zbreak/hardcopy */
	tpara.c_cc[VMIN] = 1;
	tpara.c_cc[VTIME] = 1;
    }
    ioctl (0, TCSETA, &tpara);
    return;
}					/* end of set_io() */
/******************************************************************************/
void
set_break (break_char)
	short   break_char;		/* -1  = break disabled */

					/* ETX = MUMPS break    */
					/* DEL = UNIX quit      */
{
    struct termio arg;

    ioctl (0, TCGETA, &arg);
    arg.c_cc[VINTR] = break_char;	/* interrupt = CTRL/C   */
    ioctl (0, TCSETA, &arg);
    return;
}					/* end of set_break() */
/******************************************************************************/
void
set_zbreak (quit_char)
	short   quit_char;		/* -1  = quit disabled */

					/* STX = MUMPS CTRL/B   */
					/* DEL = UNIX quit      */
{
    struct termio arg;

    ioctl (0, TCGETA, &arg);
    arg.c_cc[VQUIT] = quit_char;
    ioctl (0, TCSETA, &arg);
    return;
}					/* end of set_zbreak() */
#else
void
set_io (action)				/* set io_parameters to mumps/unix conventions */
	short   action;			/* 0 = UNIX; 1 = MUMPS */
{
    static struct termios unixtermio;
    struct termios termios_p;

    if (filter)
	return;				/* nothing special if used as
					 * filter */
    if (action == UNIX)
	tcsetattr (0, TCSADRAIN, &unixtermio);
    else {				/* action == MUMPS */
	tcgetattr (0, &unixtermio);	/* get unix paramters */
	tcgetattr (0, &termios_p);	/* get paramters */
	termios_p.c_lflag &= (~(ECHO | ICANON));	/* disable echo/cbreak mode */
	termios_p.c_iflag &= ~ICRNL;	/* cr-lf mapping */
	termios_p.c_oflag &= ~ONLCR;	/* cr-lf mapping */
	termios_p.c_cc[VINTR] = BrkKey;	/* interrupt = CTRL/C */
	termios_p.c_cc[VQUIT] = (zbreakon || hardcopy) ? STX : -1;	/* zbreak/hardcopy */
	termios_p.c_cc[VMIN] = 1;
	termios_p.c_cc[VTIME] = 1;
	tcsetattr (0, TCSADRAIN, &termios_p);	/* set paramters */
    }
    return;
}					/* end of set_io() */
/******************************************************************************/
void
set_break (break_char)
	short   break_char;		/* -1  = break disabled */

					/* ETX = MUMPS break    */
					/* DEL = UNIX quit      */
{
    struct termios termios_p;

    tcgetattr (0, &termios_p);
    termios_p.c_cc[VINTR] = break_char;	/* interrupt = CTRL/C   */
    tcsetattr (0, TCSADRAIN, &termios_p);	/* set paramters */
    return;
}					/* end of set_break() */
/******************************************************************************/
void
set_zbreak (quit_char)
	short   quit_char;		/* -1  = quit disabled */

					/* STX = MUMPS CTRL/B   */
					/* DEL = UNIX quit      */
{
    struct termios termios_p;

    tcgetattr (0, &termios_p);
    termios_p.c_cc[VQUIT] = quit_char;
    tcsetattr (0, TCSADRAIN, &termios_p);	/* set paramters */
    return;
}					/* end of set_zbreak() */
#endif /* OLDUNIX */
/******************************************************************************/
#else
 /* same for XENIX */
void
set_io (action)				/* set io_parameters to mumps/unix conventions */
	short   action;			/* 0 = UNIX; 1 = MUMPS */
{
    static char del;
    static char fs;
    struct sgttyb tt;
    struct tchars tc;

    if (filter)
	return;				/* nothing special if used as
					 * filter */
    fflush (stdin);
    fflush (stdout);
    fflush (stderr);
    ioctl (0, TIOCGETP, &tt);
    ioctl (0, TIOCGETC, &tc);
    if (action == UNIX) {
	tt.sg_flags &= ~02;		/* no cbreak mode           */
	tt.sg_flags |= (010 | 020);	/* echo ; CR to LF map      */
	tc.t_quitc = fs;		/* quit                     */
	tc.t_intrc = del;		/* interrupt                */
/* we desperately tried to do it with DEL - but it didn't work */
    } else {				/* action == MUMPS */
	tt.sg_flags |= 02;		/* cbreak mode              */
	tt.sg_flags &= (~010 & ~020);	/* 10 no echo; 20 no CR map */
	del = tc.t_intrc;		/* save previous state of   */
	fs = tc.t_quitc;		/* quit and interrupt       */
	tc.t_intrc = BrkKey;		/* interrupt = CTRL/C       */
	tc.t_quitc = (zbreakon || hardcopy) ? STX : -1;		/* zbreak/hardcopy */
    }
    ioctl (0, TIOCSETP, &tt);
    ioctl (0, TIOCSETC, &tc);
}					/* end of set_io() */
/******************************************************************************/
void
set_break (break_char)
	short   break_char;		/* -1  = break disabled */

					/* ETX = MUMPS break    */
					/* DEL = UNIX quit      */
{
    struct tchars tc;

    ioctl (0, TIOCGETC, &tc);
    tc.t_intrc = break_char;
    ioctl (0, TIOCSETC, &tc);
    return;
}					/* end of set_break */
/******************************************************************************/
void
set_zbreak (quit_char)
	short   quit_char;		/* -1  = quit disabled */

					/* STX = MUMPS CTRL/B   */
					/* DEL = UNIX quit      */
{
    struct tchars tc;

    ioctl (0, TIOCGETC, &tc);
    tc.t_quitc = quit_char;
    ioctl (0, TIOCSETC, &tc);
    return;
}					/* end of set_zbreak */
#endif /* SYSFIVE */
/******************************************************************************/
void
zload (rou)				/* load routine in buffer */
	char   *rou;

{
    FILE   *infile;
    short   linelen;
    char    pgm[256];
    char    tmp1[256];

    register long int i,
            j,
            ch;

    char   *savptr;			/* save routine pointer */
    long    timex;
    short   altern;

/* Routines are stored in routine buffers. If a routine is called
 * we first look whether it's already loaded. If not, we look for
 * the least recently used buffer and load it there. Besides
 * dramatically improved performance there is little effect on
 * the user. Sometimes you see an effect: if the program is changed
 * by some other user or by yourself using the 'ced' editor you
 * may get the old version for some time with DO, GOTO or ZLOAD.
 * A ZREMOVE makes sure the routine is loaded from disk.
 */
    if (*rou == EOL || *rou == 0) {	/* routine name empty */
	pgms[0][0] = EOL;
	rouend = rouins = rouptr = buff;
	roucur = buff + (NO_OF_RBUF * PSIZE0 + 1);
	*rouptr = EOL;
	*(rouptr + 1) = EOL;
	*(rouptr + 2) = EOL;
	dosave[0] = 0;
	return;
    }
    savptr = rouptr;
/* what time is it ? */
    timex = time (0L);
/* let's have a look whether we already have the stuff */
    for (i = 0; i < NO_OF_RBUF; i++) {
	if (pgms[i][0] == 0) {
	    altern = i;
	    break;
	}				/* buffer empty */
	j = 0;
	while (rou[j] == pgms[i][j]) {
	    if (rou[j++] == EOL) {
		rouptr = buff + (i * PSIZE0);
		ages[i] = time (0L);
		rouend = ends[i];
		rouins = rouend - 1;
		return;
	    }
	}
	if (ages[i] <= timex)
	    timex = ages[altern = i];
    }
/* we don't have it. 'altern' is the least recently used buffer */
    if (rgafile[0] != EOL) {
	char    tmp[256];
	short   x;

	tmp[x = stcpy (tmp, rou)] = LF;
	tmp[++x] = NUL;
	fputs (tmp, rgaccess);
    }
/* clear DO-label stored under FOR */
    dosave[0] = 0;
    j = 0;
    ch = EOL;				/* init for multiple path search */
    tmp1[0] = EOL;
  nextpath:				/* entry point for retry */
    i = 0;
    if (rou[0] == '%') {		/* %_routines are in special directory */
	if (mcmnd >= 'a') {		/* DO GOTO JOB */
	    if (rou0plib[j] != EOL)
		while ((ch = pgm[i++] = rou0plib[j++]) != ':' && ch != EOL) ;
	} else if (rou1plib[j] != EOL)
	    while ((ch = pgm[i++] = rou1plib[j++]) != ':' && ch != EOL) ;
    } else {
	if (mcmnd >= 'a') {		/* DO GOTO JOB */
	    if (rou0path[j] != EOL)
		while ((ch = pgm[i++] = rou0path[j++]) != ':' && ch != EOL) ;
	} else if (rou1path[j] != EOL)
	    while ((ch = pgm[i++] = rou1path[j++]) != ':' && ch != EOL) ;
    }
    if (i > 0) {
	if (i == 1 || (i == 2 && pgm[0] == '.'))
	    i = 0;
	else
	    pgm[i - 1] = '/';
    }
    pgm[i] = EOL;
    stcpy (tmp1, pgm);			/* directory where we search for the routine */
    stcpy (&pgm[i], rou);
    rouptr = buff + (altern * PSIZE0);
    stcat (pgm, rou_ext);
    pgm[stlen (pgm)] = NUL;		/* append routine extention */

    if ((infile = fopen (pgm, "r")) == NULL) {
	rouptr = savptr;
	if (ch != EOL)
	    goto nextpath;		/* try next access path */
	stcpy (varerr, rou);
	ierr = NOPGM;
	return;
    }
  again:;
    linelen = 0;
    savptr = rouend = rouptr;
    for (i = 1; i < (PSIZE0 - 1); i++) {
	*++rouend = ch = getc (infile);
	if (ch == LF || ch == EOF) {
	    *rouend++ = EOL;
	    i++;
	    *savptr = i - linelen - 2;
	    savptr = rouend;
	    linelen = i;
	    if (ch == EOF) {
		fclose (infile);
		*rouend-- = EOL;
		rouins = rouend - 1;
		ends[altern] = rouend;
		ages[altern] = time (0L);
		stcpy (pgms[altern], rou);
		stcpy (path[altern], tmp1);
		return;
	    }
	}
    }
    rouptr = savptr;
    if (autorsize) {
	while ((ch = getc (infile)) != EOF) {
	    i++;
	    if (ch == LF)
		i++;
	}				/* how big? */
	i = ((i + 3) & ~01777) + 02000;	/* round for full kB; */
	if (newrsize (i, NO_OF_RBUF) == 0) {	/* try to get more routine space. */
	    altern = 0;
	    ch = EOL;
	    fseek (infile, 0L, 0);
	    goto again;
	}
    }
    fclose (infile);
    goto pgmov;
  pgmov:;
/* program overflow error */
    rouptr = rouins = rouend = savptr;
    (*savptr++) = EOL;
    *savptr = EOL;
    for (i = 0; i < NO_OF_RBUF; i++) {
	ages[i] = 0;
	pgms[i][0] = 0;
    }
    pgms[i][0] = EOL;
    rou_name[0] = EOL;
    ierr = PGMOV;
    return;
}					/* end of zload() */
/******************************************************************************/
void
zsave (rou)				/* save routine on disk */
	char   *rou;

{
    register i;
    register j;
    register ch;
    char    tmp[256];

    stcpy (tmp, rou);			/* save name without path */

/* look whether we know where the routine came from */
    if (zsavestrategy) {		/* VIEW 133: remember ZLOAD directory on ZSAVE */
	for (i = 0; i < NO_OF_RBUF; i++) {
	    if (pgms[i][0] == 0)
		break;			/* buffer empty */
	    j = 0;
	    while (rou[j] == pgms[i][j]) {
		if (rou[j++] == EOL) {
		    stcpy (rou, path[i]);
		    stcat (rou, tmp);
		    j = 0;
		    ch = 1;		/* init for multiple path search */
		    goto try;
		}
	    }
	}
    }
/* not found */
    j = 0;
    ch = EOL;				/* init for multiple path search */
  nextpath:				/* entry point for retry */
    if (tmp[0] == '%') {
	if (rou1plib[0] != EOL) {
	    i = 0;
	    while ((ch = rou[i++] = rou1plib[j++]) != ':' && ch != EOL) ;
	    if (i == 1 || (i == 2 && rou[0] == '.'))
		i = 0;
	    else
		rou[i - 1] = '/';
	    stcpy (&rou[i], tmp);
	}
    } else {
	if (rou1path[0] != EOL) {
	    i = 0;
	    while ((ch = rou[i++] = rou1path[j++]) != ':' && ch != EOL) ;
	    if (i == 1 || (i == 2 && rou[0] == '.'))
		i = 0;
	    else
		rou[i - 1] = '/';
	    stcpy (&rou[i], tmp);
	}
    }
  try:;

    stcat (rou, rou_ext);
    rou[stlen (rou)] = NUL;		/* append routine extention */

    if (rouend <= rouptr) {
	unlink (rou);
	rou_name[0] = EOL;
    } else {
	FILE   *outfile;
	char   *i0;

	for (;;)
	{
	    errno = 0;
	    if ((outfile = fopen (rou, "w")) != NULL)
		break;
	    if (errno == EINTR)
		continue;		/* interrupt */
	    if (errno == EMFILE || errno == ENFILE) {
		close_all_globals ();
		continue;
	    }				/* free file_des */
	    if (ch != EOL)
		goto nextpath;		/* try next access path */
	    ierr = PROTECT;
	    return;
	}
	i0 = rouptr;
	while (++i0 < (rouend - 1)) {
	    if ((ch = (*(i0))) == EOL) {
		ch = LF;
		i0++;
	    }
	    putc (ch, outfile);
	}
	if (ch != LF)
	    putc (LF, outfile);
	fclose (outfile);
    }
    return;
}					/* end of zsave() */
/******************************************************************************/
void
lock (lockarg, time_out, type)		/* execution of the LOCK command */
	char   *lockarg;		/* lockarg[0] = lockmode         */

				     /*            SP=normal          */
				     /*            '+'=incremental    */
				     /*            '-'=decremental    */
				     /* &lockarg[1]=name(s) to be (un)locked */
	long    time_out;
	char    type;			/* l=LOCK, A=ZALLOCATE, D=ZDEALLOCATE */

				     /* j=JOB (clear nolocks flag          */
{
/* nolocks and noallox are TRUE if            */
/* the respective table is guaranteed         */
/* to contain no entries for the current job  */
    static short nolocks = TRUE;	/* empty flag locks */
    static short noallox = TRUE;	/* empty flag allocates */
    short   ltab;			/* file descr. for locktab */
    int     cpid;
    short   scoop;			/* flag to retain entries */
    char    entry[256];
    long int r_pos;			/* position to read  */
    long int w_pos;			/* position to write */
    int     i,
            j;
    int     found;


/* after JOB our table is supposed to be empty of PIDs entries */
    if (type == 'j') {
	nolocks = TRUE;
	noallox = TRUE;
	return;
    }
/* after SET $JOB the table is possibly not empty of PIDs entries */
    if (type == 's') {
	nolocks = FALSE;
	noallox = FALSE;
	return;
    }
    if (lockarg[1] == EOL) {		/* free LOCKs in empty table: nothing to do! */
	if (type == 'l') {
	    if (nolocks)
		return;
	} else {
	    if (noallox)
		return;
	}
    }
/*      if 'locktab' does not exist, create it */
    for (;;)
    {
	errno = 0;
	if ((ltab = open (type == 'l' ? locktab : zallotab, 2)) != -1)
	    break;
	if (errno == EINTR)
	    continue;			/* signal interrupt */
	if (errno == ENOENT) {
	    if (lockarg[1] == EOL)
		return;			/* nothing to do */
	    ltab = creat (type == 'l' ? locktab : zallotab, 0666);
	    if (errno == EMFILE || errno == ENFILE) {
		close_all_globals ();
		ltab = creat (type == 'l' ? locktab : zallotab, 0666);
	    }
	    if (ltab == -1) {
		ierr = PROTECT;
		return;
	    }
	}
	entry[0] = 0;
	entry[1] = 0;			/* zero word is terminator of locktab */
	lseek (ltab, 0L, 0);
	write (ltab, entry, 2);
	close (ltab);
	if (ierr == INRPT)
	    return;
    }

/*      request exclusive access to locktab (read & write) */
    locking (ltab, 1, 0L);

/*      free all of your own locks; we do it by copying the whole stuff */
/*      within 'locktab' omitting the old entries under our PID         */
    j = 0;				/* count your old entries */
    lseek (ltab, 0L, 0);
    w_pos = 0L;
    for (;;)
    {
	read (ltab, entry, 2);
	cpid = UNSIGN (entry[0]) * 256 + UNSIGN (entry[1]);
	if (cpid == 0) {
	    lseek (ltab, w_pos, 0);
	    write (ltab, entry, 2);
	    break;
	}
	i = 1;
	do {
	    read (ltab, &entry[++i], 1);
	} while (entry[i] != EOL);
	i++;
	scoop = FALSE;
	if (cpid != pid)
	    scoop = TRUE;
	else if (entry[2] != type)
	    scoop = TRUE;
	else if (lockarg[0] == '+')
	    scoop = TRUE;
	else if (lockarg[0] == '-') {
	    int     k;

	    k = 0;
	    if ((found = find (&lockarg[1], &entry[3])))
		while (lockarg[found + k] == entry[3 + k])
		    k++;
	    if (k && (lockarg[found + k] == 1)) {
		lockarg[found] = '!';
	    }
/* unlock only once */
	    else
		scoop = TRUE;
	} else if (type == 'D' &&
		   lockarg[1] != EOL &&
		   stcmp (&lockarg[1], &entry[3]))
	    scoop = TRUE;

	if (scoop) {
	    if (j) {
		r_pos = tell (ltab);
		lseek (ltab, w_pos, 0);
		write (ltab, entry, (unsigned) i);
		lseek (ltab, r_pos, 0);
	    }
	    w_pos += i;
	}
	if (cpid == pid)
	    j++;
    }
    locking (ltab, 0, 0L);
    close (ltab);
    if (lockarg[1] == EOL) {		/* it was an argumentless LOCK */
	if (type == 'l')
	    nolocks = TRUE;
	else
	    noallox = TRUE;
	return;
    }
    if (lockarg[0] == '-') {		/* it was a decremental LOCK */
	if (time_out >= 0L)
	    test = TRUE;
	return;
    }					/* always TRUE */
    if (type == 'D')
	return;				/* it was a ZDEALLOCATE */

    if (type == 'l')
	nolocks = FALSE;		/* reset empty flag */
    else
	noallox = FALSE;

    for (;;)
    {					/* request 'locktab' */
	while ((ltab = open (type == 'l' ? locktab : zallotab, 2)) == -1) ;
	locking (ltab, 1, 0L);
	lseek (ltab, 0L, 0);

/* if (requested entries are not in 'locktab') break; */
	for (;;)
	{
	    if (read (ltab, entry, 2) < 2)
		break;			/* pid */
	    if ((cpid = UNSIGN (entry[0]) * 256 + UNSIGN (entry[1])) == 0)
		break;
	    i = 0;
	    while (read (ltab, &entry[i], 1) == 1 && entry[i] != EOL)
		i++;
	    if (cpid == pid)
		continue;		/* ZALLOCATE !! */
	    j = 1;
	    while (lockarg[j] != EOL) {
		i = 1;
		while (lockarg[j] == entry[i]) {
		    if (entry[i] == EOL)
			break;
		    i++;
		    j++;
		}

/* 01/18/99 rlf Not sure if this is a libc-6 issue or not, but compiler */
/*              was throwing a warning about each of the lockarg[j]>DEL */
/*              comparisons, saying "warning:comparison is always 0 due */
/*              to a limited range of data types". */

#ifdef LINUX_GLIBC
  if ((lockarg[j]<2 || UNSIGN(lockarg[j])>DEL) &&
  (entry[i]<2 || UNSIGN(entry[i])>DEL)) goto found;
#else
  if ((lockarg[j]<2 || lockarg[j]>DEL) &&
  (entry  [i]<2 || entry  [i]>DEL)) goto found;
#endif /* LINUX_GLIBC */

		while (lockarg[j++] != 1) ;	/* skip to next piece */
	    }
	    if (ierr == INRPT) {
		locking (ltab, 0, 0L);
		close (ltab);
		return;
	    }
	}
	w_pos = tell (ltab) - 2;
	goto notfound;

/* release and wait if timeout not expired */
      found:;
	locking (ltab, 0, 0L);
	close (ltab);
	if (time_out >= 0L) {
	    if (--time_out < 0L) {
		test = FALSE;
		return;
	    }
	}
	if (ierr == INRPT)
	    return;
	sleep (1);

    }

  notfound:				/* enter your own entries and release 'locktab' */
    i = w_pos;
    i = 1;
    lseek (ltab, w_pos, 0);
    while (lockarg[i] != EOL) {
	entry[0] = pid / 256;
	entry[1] = pid % 256;
	entry[2] = type == 'l' ? 'l' : 'D';
	write (ltab, entry, 3);
	j = i;
	while (lockarg[++i] != 1) ;
	lockarg[i++] = EOL;
	write (ltab, &lockarg[j], (unsigned) (i - j));
    }
    entry[0] = 0;
    entry[1] = 0;
    write (ltab, entry, 2);		/* zero pid as terminator */
    locking (ltab, 0, 0L);
    close (ltab);
    if (time_out >= 0L)
	test = TRUE;
    return;
}					/* end of lock */
/******************************************************************************/
void
getraddress (a, lvl)			/* returns the 'canonical' address of the line */

/* at the specified DO/FOR/XEC level           */
	char   *a;			/* result: pointer to 'address' */
	short   lvl;			/* process this level           */
{
    char   *rcur;			/* cursor into routine         */
    short   f;
    char    tmp3[256];
    char   *j0;
    char   *j1;
    short   rlvl;			/* lower level, where to find routine name */
    register i,
            j;

    f = mcmnd;
    mcmnd = 'd';			/* make load use standard-path */
    rlvl = lvl;
    if (nestn[rlvl] == 0 && rlvl < nstx)
	rlvl++;
    if (nestn[rlvl])
	zload (nestn[rlvl]);
    mcmnd = f;
/* command on stack: 2 == DO_BLOCK; other: make uppercase */
    i = nestc[lvl];
    if (i != '$')
	i = ((i == 2) ? 'd' : i - 32);
    a[0] = '(';
    a[1] = i;
    a[2] = ')';
    a[3] = EOL;				/* command */

    rcur = nestr[lvl] + rouptr;		/* restore rcur */
    j0 = (rouptr - 1);
    j = 0;
    tmp3[0] = EOL;
    j0++;
    if (rcur < rouend) {
	while (j0 < (rcur - 1)) {
	    j1 = j0++;
	    j++;
	    if ((*j0 != TAB) && (*j0 != SP)) {
		j = 0;
		while ((tmp3[j] = (*(j0++))) > SP) {
		    if (tmp3[j] == '(')
			tmp3[j] = EOL;
		    j++;
		}
		tmp3[j] = EOL;
		j = 0;
	    }
	    j0 = j1;
	    j0 += (UNSIGN (*j1)) + 2;
	}
    }
    stcat (a, tmp3);
    if (j > 0) {
	i = stlen (a);
	a[i++] = '+';
	intstr (&a[i], j);
    }
    if (nestn[rlvl]) {
	stcat (a, "^\201");
	stcat (a, nestn[rlvl]);
    } else if (rou_name[0] != EOL) {
	stcat (a, "^\201");
	stcat (a, rou_name);
    }
    f = mcmnd;
    mcmnd = 'd';			/* make load use standard-path */
    zload (rou_name);
    mcmnd = f;
    return;
}					/* end getraddress() */
/******************************************************************************/

/* End of $Source: /cvsroot-fuse/gump/FreeM/src/service.c,v $ */
