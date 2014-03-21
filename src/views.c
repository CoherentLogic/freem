/***
 * $Source: /cvsroot-fuse/gump/FreeM/src/views.c,v $
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
 * VIEW command and $VIEW function
 * 
 */

#include "mpsdef.h"
#define LOCK        'l'
#define ZDEALLOCATE 'D'

/* system services */

#include <signal.h>

#ifdef FREEBSD
#include <termios.h>
#define TCGETA TIOCGETA
#define TCSETA TIOCSETA
#define termio termios
#else
#include <termio.h>
#endif

#ifdef __CYGWIN__
#include <errno.h>
#endif /* __CYGWIN__ */
#include <errno.h> //jpw
  
FILE   *popen ();
char   *calloc ();


char   *strcpy ();
void    free ();

#ifndef FREEBSD
long int lseek ();
#endif

/* 01/18/99 rlf Apparently, tell disappeared with libc-6 */
#ifdef LINUX_GLIBC
long int
tell (int fd)
{
    return lseek (fd, 0, SEEK_CUR);
}
#else
long int
tell ()
#endif					/* LINUX_GLIBC */

/******************************************************************************/
	void    view_com ()
{					/* process VIEW command */
    char    tmp[256],
            tmp2[256];
    int     arg1;
    register long int i,
            j,
            ch;

    if (*codptr == SP || *codptr == EOL) {	/* no argument form of VIEW */
	ierr = ARGER;
	return;
    }
    expr (STRING);
    arg1 = intexpr (argptr);
    if (ierr > OK)
	return;
    if (*codptr == ':') {
	codptr++;
	expr (STRING);
	if (ierr > OK)
	    return;
	switch (arg1) {
	case 0:			/* VIEW 0: intexpr is set prio to 'intexpr' */
	    i = nice (40);
	    j = nice ((int) (intexpr (argptr) - 40));
	    return;
/* VIEW 1: set Device Status word */
	case 1:
	    i = intexpr (argptr);
	    if (ierr != MXNUM)
		DSW = i;
	    return;
	case 2:			/* VIEW 2: set current directory */
	    if (*argptr != EOL) {
		tmp[stcpy (tmp, argptr)] = NUL;
		if (chdir (tmp))
		    ierr = PROTECT;
		else
		    strcpy (curdir, tmp);
	    }
	    for (i = 0; i < NO_OF_RBUF; i++) {	/* empty routine buffer */
		pgms[i][0] = EOL;
		ages[i] = 0L;
	    }
	    close_all_globals ();		/* close globals */
	    return;
	case 3:			/* VIEW 3: set global access path */
	    if (stlen (argptr) >= PATHLEN) {
		ierr = MXSTR;
		return;
	    }
	    stcpy (glopath, argptr);
	    for (i = 0; i < NO_GLOBLS; i++)
		if (oldfil[i][0] > '%') {	/* close non% globals */
		    close (olddes[i]);
		    usage[i] = 0;
		    olddes[i] = 0;
		    oldfil[i][0] = NUL;
		}
	    return;
	case 4:			/* VIEW 4: DO-GOTO-JOB routine access path */
	    if (stlen (argptr) >= PATHLEN) {
		ierr = MXSTR;
		return;
	    }
	    stcpy (rou0path, argptr);
	    for (i = 0; i < NO_OF_RBUF; i++)	/* empty routine buffer */
		if (pgms[i][0] != '%') {
		    if (rouptr != (buff + (i * PSIZE0))) {	/* all except current routine */
			pgms[i][0] = EOL;
			ages[i] = 0L;
		    }
		    path[i][0] = EOL;
		}
	    return;
	case 5:			/* VIEW 5: ZL-ZS routine access path */
	    if (stlen (argptr) >= PATHLEN) {
		ierr = MXSTR;
		return;
	    }
	    stcpy (rou1path, argptr);
	    for (i = 0; i < NO_OF_RBUF; i++)	/* empty routine buffer */
		if (pgms[i][0] != '%') {
		    if (rouptr != (buff + (i * PSIZE0))) {	/* all except current routine */
			pgms[i][0] = EOL;
			ages[i] = 0L;
		    }
		    path[i][0] = EOL;
		}
	    return;
	case 6:			/* VIEW 6: %global access path */
	    if (stlen (argptr) >= PATHLEN) {
		ierr = MXSTR;
		return;
	    }
	    stcpy (gloplib, argptr);
	    for (i = 0; i < NO_GLOBLS; i++)	/* close % globals */
		if (oldfil[i][0] == '%') {
		    close (olddes[i]);
		    usage[i] = 0;
		    olddes[i] = 0;
		    oldfil[i][0] = NUL;
		}
	    return;
	case 7:			/* VIEW 7: %_routine DO-GOTO-JOB access path */
	    if (stlen (argptr) >= PATHLEN) {
		ierr = MXSTR;
		return;
	    }
	    stcpy (rou0plib, argptr);
	    for (i = 0; i < NO_OF_RBUF; i++)	/* empty routine buffer */
		if (pgms[i][0] == '%') {
		    if (rouptr != (buff + (i * PSIZE0))) {	/* all except current routine */
			pgms[i][0] = EOL;
			ages[i] = 0L;
		    }
		    path[i][0] = EOL;
		}
	    return;
	case 8:			/* VIEW 8: %_routine ZL-ZS access path */
	    if (stlen (argptr) >= PATHLEN) {
		ierr = MXSTR;
		return;
	    }
	    stcpy (rou1plib, argptr);
	    for (i = 0; i < NO_OF_RBUF; i++)	/* empty routine buffer */
		if (pgms[i][0] == '%') {
		    if (rouptr != (buff + (i * PSIZE0))) {	/* all except current routine */
			pgms[i][0] = EOL;
			ages[i] = 0L;
		    }
		    path[i][0] = EOL;
		}
	    return;
	case 9:			/* VIEW 9: set OPEN/USE/CLOSE path */
	    if (stlen (argptr) >= PATHLEN) {
		ierr = MXSTR;
		return;
	    }
	    stcpy (oucpath, argptr);
	    return;
	case 10:			/* VIEW 10: set zallocktable ; ZDEALLOCATE */
	    if (stlen (argptr) >= PATHLEN) {
		ierr = MXSTR;
		return;
	    }
	    lock (" \201", -1, ZDEALLOCATE);	/* ZDEALLOCATE ALL */
	    zallotab[stcpy (zallotab, argptr)] = NUL;
	    return;
	case 11:			/* VIEW 11: set locktable ; UNLOCK */
	    if (stlen (argptr) >= PATHLEN) {
		ierr = MXSTR;
		return;
	    }
	    lock (" \201", -1, LOCK);	/* un-LOCK */
	    locktab[stcpy (locktab, argptr)] = NUL;
	    return;
	case 12:			/* VIEW 12: set rga protocol file */
	    if (stlen (argptr) >= PATHLEN) {
		ierr = MXSTR;
		return;
	    }
	    tmp[0] = 'a';
	    tmp[1] = NUL;
	    if (rgaccess != NULL)
		fclose (rgaccess);
	    tmp2[stcpy (tmp2, argptr)] = NUL;
	    if (tmp2[0] == NUL) {
		rgafile[0] = EOL;
		rgaccess = NULL;
	    } else if ((rgaccess = fopen (tmp2, tmp)) == NULL) {
		ierr = PROTECT;
		rgafile[0] = EOL;
	    } else
		stcpy (rgafile, argptr);
	    return;
	case 13:			/* VIEW 13: hardcopy file */
	    if (stlen (argptr) >= PATHLEN) {
		ierr = MXSTR;
		return;
	    }
	    hcpyfile[stcpy (hcpyfile, argptr)] = NUL;
	    return;
	case 14:			/* VIEW 14: set journal file */
	    if (stlen (argptr) >= PATHLEN) {
		ierr = MXSTR;
		return;
	    }
	    jourfile[stcpy (jourfile, argptr)] = NUL;
	    return;
	case 15:			/* VIEW 15: set journal flag  0=inactive 1=write -1=read */
	    i = intexpr (argptr);

/* jour_flag may already have desired value */
	    if ((i < 0 && jour_flag < 0) ||
		(i > 0 && jour_flag > 0) ||
		(i == 0 && jour_flag == 0))
		return;

	    if (i < 0) {
		i = (-1);
		ug_buf[HOME][0] = EOL;
	    }
	    if (i > 0)
		i = 1;
	    jour_flag = i;
	    if (jouraccess)
		fclose (jouraccess);
	    if (jourfile[0] == NUL) {
		goto hcend;
	    }
	    tmp[0] = jour_flag > 0 ? 'a' : 'r';
/* a=WRITE append; r=READ */
	    tmp[1] = NUL;		/* NUL not EOL !!! */
	    if ((jouraccess = fopen (jourfile, tmp)) == NULL)
		goto hcend;
	    return;
	  hcend:;
	    jour_flag = 0;
	    ierr = ARGER;
	    return;
	case 16:			/* VIEW 16: load error messages */
	    i = intexpr (argptr);
	    if (i < 1 || i >= MAXERR) {
		ierr = ARGER;
		return;
	    }
	    if (*codptr != ':') {
		ierr = ARGER;
		return;
	    }
	    codptr++;
	    expr (STRING);
	    if (stlen (argptr) > ERRLEN)
		ierr = MXSTR;
	    if (ierr > OK)
		return;
	    stcpy (errmes[i], argptr);
	    return;
	case 17:			/* VIEW 17: set intrinsic z-commands */
	    m_tolower (argptr);
	    stcpy (zcommds, argptr);
	    return;
	case 18:			/* VIEW 18: set intrinsic z-commands */
	    m_tolower (argptr);
	    stcpy (zfunctions, argptr);
	    return;
	case 19:			/* VIEW 19: set intrinsic z-commands */
	    m_tolower (argptr);
	    stcpy (zsvn, argptr);
	    return;
	case 20:			/* VIEW 20: break service code */
	    stcpy (brkaction, argptr);
	    return;
	case 22:			/* VIEW 22: alternate name for a glvn */
	    if (tstglvn (argptr) == FALSE) {
		ierr = INVREF;
		return;
	    }
/* un_define an alias */
	    if (v22ptr) {		/* there are aliases */
		int     k,
		        k1;

		i = 0;
		while (i < v22ptr) {
		    k = i;
		    k1 = i + UNSIGN (v22ali[i]) + 1;
		    j = 0;		/* is current reference an alias ??? */
		    while (v22ali[++i] == argptr[j]) {
			if (v22ali[i] == EOL)
			    break;
			j++;
		    }
/* yes, it is, so resolve it now! */
		    if (v22ali[i] == EOL && argptr[j] == EOL) {
			if (v22ptr > k1)
			    stcpy0 (&v22ali[k], &v22ali[k1], v22ptr - k1);
			if ((v22ptr -= (k1 - k)) <= 0) {
			    v22ptr = 0;
			    v22size = 0;
			    free (v22ali);
			}
			break;
		    }
		    i = k1;
		}
	    }
	    if (*codptr == ':') {	/* define an alias */
		codptr++;
		stcpy (tmp, argptr);
		expr (STRING);
		if (ierr > OK)
		    return;
/* check only locals */
		if (*argptr != '^' && tstglvn (argptr) == FALSE) {
		    ierr = INVREF;
		    return;
		}
/**********/

		if (v22ptr) {		/* check for circular definitions */
		    stcpy (tmp2, argptr);
		    procv22 (tmp2);
		    if (stcmp (tmp2, tmp) == 0)
			return;		/* defined by itself */
		}
/**********/

/* new entry to alias table. */
		i = stlen (tmp);
		j = stlen (argptr);
		if (stcmp (tmp, argptr) != 0) {		/* names are not equal */
		    if ((v22ptr + i + j + 3) >= v22size) {	/* more space needed */
			char   *newv22;

			v22size += 1024;
			while ((newv22 = calloc ((unsigned) v22size, 1)) == 0) ;
			if (v22ptr) {
			    stcpy0 (newv22, v22ali, v22ptr);
			    free (v22ali);
			}
			v22ali = newv22;
		    }
		    v22ali[v22ptr++] = (char) (i + j + 2);	/* byte for fast skipping */
		    stcpy (&v22ali[v22ptr], tmp);
		    v22ptr += (i + 1);
		    stcpy (&v22ali[v22ptr], argptr);
		    v22ptr += (j + 1);
		    return;
		}
	    }
	    return;

	case 23:			/* VIEW 23: put string in input queue */
	    if (jour_flag >= 0 || io != HOME)
		stcpy (ug_buf[io], argptr);
	    return;

	case 24:			/* VIEW 24: put string screen line */
	    i = intexpr (argptr);
	    if ((ch = (i < 0)))
		i = (-i);
	    if (i <= 0 || i > N_LINES || *codptr != ':') {
		ierr = ARGER;
		return;
	    }
	    codptr++;
	    expr (STRING);
	    if (ierr > OK)
		return;
	    if ((j = stlen (argptr)) > N_COLUMNS)
		j = N_COLUMNS;
	    if (ch)
		stcpy0 ((*screen).screena[(unsigned int) (*screen).sclines[i - 1]], argptr, j);
	    else
		stcpy0 ((*screen).screenx[(unsigned int) (*screen).sclines[i - 1]], argptr, j);
	    part_ref (screen, (short) (i - 1), (short) (i - 1));
	    return;
	    
	case 25:
	    i = intexpr (argptr);
	    if (i <= 0 || i > N_LINES || *codptr != ':') {
		ierr = ARGER;
		return;
	    }
	    codptr++;
	    expr (STRING);
	    if (ierr > OK)
		return;
	    tmp[0] = ESC;
	    tmp[1] = '[';
	    ch = 2;
	    if (i > 9)
		tmp[ch++] = '0' + (i / 10);
	    tmp[ch++] = '0' + (i % 10);
	    tmp[ch++] = 'H';
	    tmp[ch] = EOL;
	    i = xpos[HOME] + 1;
	    j = ypos[HOME] + 1;		/* save position */
/* page mode avoids scroll up */
#ifdef SCO
	    ch = (*screen).sc_lo;
	    (*screen).sc_lo = N_LINES;
#else
/* nicht fuer abges.Terminals           stcat(tmp,"\033[8u\201"); */
#endif /* SCO */
	    writeHOME (tmp);		/* set position */
	    writeHOME (argptr);
#ifdef SCO
	    (*screen).sc_lo = ch;
	    ch = 0;
#else
	    ch = 0;
/* nicht fuer abges.Terminals
 * if ((*screen).rollflag)
 * { tmp[ch++]=ESC; tmp[ch++]='[';
 * tmp[ch++]='9'; tmp[ch++]='u';
 * }   * restore scroll mode */
#endif /* SCO */
	    tmp[ch++] = ESC;
	    tmp[ch++] = '[';
/* restore position */
	    if (j > 9)
		tmp[ch++] = '0' + (j / 10);
	    tmp[ch++] = '0' + (j % 10);
	    if (i > 1) {
		tmp[ch++] = ';';
		if (i > 9)
		    tmp[ch++] = '0' + (i / 10);
		tmp[ch++] = '0' + (i % 10);
	    }
	    tmp[ch++] = 'H';
	    tmp[ch++] = (((*screen).att & 01) ? SO : SI);	/* restore SI/SO state */
	    tmp[ch++] = ESC;		/* restore SGR state */
	    tmp[ch++] = '[';
	    for (i = 1; i < 8; i++) {
		if ((*screen).att & (1 << i)) {
#ifdef SCO
		    if (i == 1) {
			tmp[ch++] = '1';
			tmp[ch++] = ';';
			continue;
		    }
#endif /* SCO */
		    tmp[ch++] = '1' + i;
		    tmp[ch++] = ';';
		}
	    }
	    if (tmp[ch - 1] == ';')
		ch--;
	    tmp[ch++] = 'm';
	    tmp[ch] = EOL;
	    writeHOME (tmp);
	    return;
	case 26:			/* VIEW 26: reset stack: multiple QUIT */
	    i = intexpr (argptr);
	    if (i < 0 || i > nstx)
		ierr = ARGER;
	    else if (i != nstx)
		repQUIT = nstx - i;
	    return;
/* VIEW 32: routine buffer size */
	case 32:
	    if ((i = intexpr (argptr)) <= 0)
		ierr = ARGER;
	    if (ierr > 0)
		return;
	    newrsize (++i, NO_OF_RBUF);
	    return;
/* VIEW 33: number of routine buffers */
	case 33:
	    if ((i = intexpr (argptr)) <= 0)
		ierr = ARGER;
	    if (ierr > 0)
		return;
	    newrsize (PSIZE0, i);
	    return;
/* VIEW 34: routine buffer size auto adjust */
	case 34:
	    autorsize = tvexpr (argptr);
	    return;
/* VIEW 37: size of 'partition', local symbols + intermed.results */
	case 37:
	    if ((i = intexpr (argptr)) <= 0)
		ierr = ARGER;
	    if (ierr > 0)
		return;
	    newpsize (i);
	    return;
/* VIEW 38: partition autoincrement flag */
	case 38:
	    autopsize = tvexpr (argptr);
	    return;
/* VIEW 39: size of userdef. special var.table */
	case 39:
	    if ((i = intexpr (argptr)) <= 0)
		ierr = ARGER;
	    if (ierr > 0)
		return;
	    newusize (i);
	    return;
/* VIEW 40: partition autoincrement flag */
	case 40:
	    autousize = tvexpr (argptr);
	    return;
/* VIEW 44: number of characters that make a name unique */
	case 44:
	    i = intexpr (argptr) & 0377;
	    if (i == 255)
		i = 0;
	    if (ierr != MXNUM)
		glvnflag.one[0] = (char) i;
	    return;
/* VIEW 45: name case sensitivity flag */
	case 45:
	    i = tvexpr (argptr);
	    toggle (i);
	    if (ierr != MXNUM)
		glvnflag.one[1] = i;
	    return;
/* VIEW 46: maximum length of name plus subscripts */
	case 46:
	    i = intexpr (argptr) & 0377;
	    if (i == 255)
		i = 0;
	    if (ierr != MXNUM)
		glvnflag.one[2] = (char) i;
	    return;
/* VIEW 47: maximum length of a subscript */
	case 47:
	    i = intexpr (argptr) & 0377;
	    if (i == 255)
		i = 0;
	    if (ierr != MXNUM)
		glvnflag.one[3] = (char) i;
	    return;
/* VIEW 48: single user flag (globals&LOCK) */
	case 48:
	    lonelyflag = tvexpr (argptr);
	    close_all_globals ();
	    break;
/* VIEW 49: lower case everywhere flag */
	case 49:
	    lowerflag = tvexpr (argptr);
	    break;
/* VIEW 50: direct mode prompt expression */
	case 50:
	    stcpy (prompt, argptr);
	    break;
/* VIEW 51: default direct mode prompt string */
	case 51:
	    stcpy (defprompt, argptr);
	    break;
/* VIEW 52: G0 input translation table */
	case 52:
	    stcpy0 (G0I[io], argptr, 256L);
	    for (i = 0; i < 256; i++) {
		if (G0I[io][i] == EOL) {
		    while (i < 256) {
			G0I[io][i] = (char) i;
			i++;
		    } break;
		}
	    }
	    break;
/* VIEW 53: G0 output translation table */
	case 53:
	    stcpy0 (G0O[io], argptr, 256L);
	    for (i = 0; i < 256; i++) {
		if (G0O[io][i] == EOL) {
		    while (i < 256) {
			G0O[io][i] = (char) i;
			i++;
		    } break;
		}
	    }
	    break;
/* VIEW 54: G1 input translation table */
	case 54:
	    stcpy0 (G1I[io], argptr, 256L);
	    for (i = 0; i < 256; i++) {
		if (G1I[io][i] == EOL) {
		    while (i < 256) {
			G1I[io][i] = (char) i;
			i++;
		    } break;
		}
	    }
	    break;
/* VIEW 55: G1 output translation table */
	case 55:
	    stcpy0 (G1O[io], argptr, 256L);
	    for (i = 0; i < 256; i++) {
		if (G1O[io][i] == EOL) {
		    while (i < 256) {
			G1O[io][i] = (char) i;
			i++;
		    } break;
		}
	    }
	    break;
/* VIEW 62: random: seed number */
	case 62:
	    i = intexpr (argptr);
	    if (ierr == MXNUM)
		return;
	    if (i < 0)
		ierr = ARGER;
	    else
		random = i;
	    break;
/* VIEW 63: random: parameter a */
	case 63:
	    i = intexpr (argptr);
	    if (ierr == MXNUM)
		return;
	    if (i <= 0)
		ierr = ARGER;
	    else
		ran_a = i;
	    break;
/* VIEW 64: random: parameter b */
	case 64:
	    i = intexpr (argptr);
	    if (ierr == MXNUM)
		return;
	    if (i < 0)
		ierr = ARGER;
	    else
		ran_b = i;
	    break;
/* VIEW 65: random: parameter c */
	case 65:
	    i = intexpr (argptr);
	    if (ierr == MXNUM)
		return;
	    if (i <= 0)
		ierr = ARGER;
	    else
		ran_c = i;
	    break;
/* VIEW 66: SIGTERM handling flag */
	case 66:
	    killerflag = tvexpr (argptr);
	    break;
/* VIEW 67: SIGHUP handling flag */
	case 67:
	    huperflag = tvexpr (argptr);
	    break;
/* ... reserved ... */
/* VIEW 70: ZSORT/ZSYNTAX flag */
	case 70:
	    s_fun_flag = tvexpr (argptr);
	    break;
/* VIEW 71: ZNEXT/ZNAME flag */
	case 71:
	    n_fun_flag = tvexpr (argptr);
	    break;
/* VIEW 72: ZPREVIOUS/ZPIECE flag */
	case 72:
	    p_fun_flag = tvexpr (argptr);
	    break;
/* VIEW 73: ZDATA/ZDATE flag */
	case 73:
	    d_fun_flag = tvexpr (argptr);
	    break;

/* VIEW 79: old ZJOB vs. new ZJOB flag */
	case 79:
	    zjobflag = tvexpr (argptr);
	    break;

/* VIEW 80: 7 vs. 8 bit flag */
	case 80:
	    eightbit = tvexpr (argptr);
	    break;

/* VIEW 81: PF1 flag */
	case 81:
	    PF1flag = tvexpr (argptr);
	    break;

/* VIEW 82: not used */
/* VIEW 83: text in $ZE flag */
	case 83:
	    etxtflag = tvexpr (argptr);
	    break;
/* VIEW 84: not used */
/* VIEW 85: not used */
/* VIEW 86: not used */

	case 87:			/* VIEW 87: date type definition */
	    i = intexpr (argptr);
	    if (i < 0 || i >= NO_DATETYPE) {
		ierr = ARGER;
		return;
	    }
	    if (*codptr != ':') {
		datetype = i;
		break;
	    }
	    if (i == 0) {
		ierr = ARGER;
		return;
	    }
	    codptr++;
	    expr (STRING);
	    j = intexpr (argptr);
	    if (*codptr != ':') {
		ierr = ARGER;
		return;
	    }
	    codptr++;
	    expr (STRING);
	    if (j > 0 && j < 15 && stlen (argptr) > MONTH_LEN)
		ierr = MXSTR;
	    else if (j > 0 && j < 13)
		stcpy (month[i][j - 1], argptr);
	    else if (j == 13)
		stcpy (dat1char[i], argptr);
	    else if (j == 14)
		stcpy (dat2char[i], argptr);
	    else if (j == 15)
		dat3char[i] = (*argptr);
	    else if (j == 16) {
		if ((j = intexpr (argptr)) < 0 || j > 2) {
		    ierr = ARGER;
		    return;
		}
		dat4flag[i] = j;
	    } else if (j == 17)
		dat5flag[i] = tvexpr (argptr);
	    else if (j == 18) {
		if ((j = intexpr (argptr) + 672411L) <= 0L) {
		    ierr = ARGER;
		    return;
		}
		datGRbeg[i] = j;
	    } else
		ierr = ARGER;
	    if (ierr > OK)
		return;
	    break;

	case 88:			/* VIEW 88: time type definition */
	    i = intexpr (argptr);
	    if (i < 0 || i >= NO_TIMETYPE) {
		ierr = ARGER;
		return;
	    }
	    if (*codptr != ':') {
		timetype = i;
		break;
	    }
	    codptr++;
	    expr (STRING);
	    j = intexpr (argptr);
	    if (*codptr != ':') {
		ierr = ARGER;
		return;
	    }
	    codptr++;
	    expr (STRING);
	    if (j == 1)
		tim1char[i] = (*argptr);
	    else if (j == 2)
		tim2char[i] = (*argptr);
	    else if (j == 3)
		tim3char[i] = (*argptr);
	    else if (j == 4)
		tim4flag[i] = tvexpr (argptr);
	    else if (j == 5)
		tim5flag[i] = tvexpr (argptr);
	    else
		ierr = ARGER;
	    if (ierr > OK)
		return;
	    break;
	case 89:			/* VIEW 89: UNDEF lvn default expression */
	    stcpy (lvndefault, argptr);
	    break;
	case 90:			/* VIEW 90: UNDEF gvn default expression */
	    stcpy (gvndefault, argptr);
	    break;
	case 91:			/* VIEW 91: missing QUIT expr default expression */
	    stcpy (exfdefault, argptr);
	    break;
	case 92:			/* VIEW 92: EUR2DEM: type mismatch error */
	    typemmflag = tvexpr (argptr);
	    break;
	case 93:			/* VIEW 93: zkey production rule definition */
	    i = intexpr (argptr);
	    if (i < 1 || i > NO_V93) {
		ierr = ARGER;
		return;
	    }
	    if (*codptr != ':') {
		v93 = i;
		break;
	    }
	    codptr++;
	    expr (STRING);
	    stcpy (v93a[i - 1], argptr);
	    break;
	case 96:			/* VIEW 96: global prefix */
	    if (stlen (argptr) > MONTH_LEN)
		ierr = MXSTR;
	    else
		stcpy (glo_prefix, argptr);
	    break;
	case 97:			/* VIEW 97: global postfix */
	    if (stlen (argptr) > MONTH_LEN)
		ierr = MXSTR;
	    else
		stcpy (glo_ext, argptr);
	    break;
	case 98:			/* VIEW 98: routine extention */
	    if (stlen (argptr) > MONTH_LEN)
		ierr = MXSTR;
	    else
		stcpy (rou_ext, argptr);
	    break;
	case 99:			/* VIEW 99: timer offset */
	    i = intexpr (argptr);
	    if (ierr > OK)
		return;
/* do not accept offset if that spells trouble within 24 hrs */
	    j = time (0L);
	    if ((tzoffset > 0 &&
		 ((j + 86400 + tzoffset) < 0 || (j + tzoffset) < 0)) ||
		(tzoffset < 0 &&
		 ((j + 86400 + tzoffset) > (j + 86400) || (j + tzoffset) > j))
		    ) {
		ierr = ARGER;
		return;
	    }
	    tzoffset = i;
	    break;
	case 100:			/* VIEW 100: send kill signal */
	    i = intexpr (argptr);
	    if (*codptr != ':') {
		j = SIGTERM;		/* SIGTERM=15 */
	    } else {
		codptr++;
		expr (STRING);
		j = intexpr (argptr);
	    }
	    if (ierr > OK)
		return;
	    if (kill (i, j))
		v100 = errno;
	    else
		v100 = 0;
	    break;
	case 110:			/* VIEW 110: local $o/$q data value */
	    stcpy (l_o_val, argptr);
	    break;
	case 111:			/* VIEW 111: global $o/$q data value */
	    stcpy (g_o_val, argptr);
	    break;
	default:
	    ierr = ARGER;
	    return;
	case 113:			/* VIEW 113: set termio infos */
	    {
		struct termio tpara;

		i = intexpr (argptr);
		if (i < 1 || i > MAXDEV)
		    ierr = NODEVICE;
		else if (devopen[i] == 0)
		    ierr = NOPEN;
		else if (*codptr != ':')
		    ierr = ARGER;
		else {
		    codptr++;
		    expr (STRING);
		    j = intexpr (argptr);
		}
		if (ierr > OK)
		    return;
		ioctl (fileno (opnfile[i]), TCGETA, &tpara);
		j = 0;
		tpara.c_iflag = intexpr (argptr);
		while ((ch = argptr[j]) != EOL) {
		    j++;
		    if (ch == ':')
			break;
		}
		tpara.c_oflag = intexpr (&argptr[j]);
		while ((ch = argptr[j]) != EOL) {
		    j++;
		    if (ch == ':')
			break;
		}
		tpara.c_cflag = intexpr (&argptr[j]);
		while ((ch = argptr[j]) != EOL) {
		    j++;
		    if (ch == ':')
			break;
		}
		tpara.c_lflag = intexpr (&argptr[j]);
		ioctl (fileno (opnfile[i]), TCSETA, &tpara);
		return;
	    }
/* VIEW 133: remember ZLOAD directory on ZSAVE */
	case 133:
	    zsavestrategy = tvexpr (argptr);
	    return;
/* VIEW 200: namespace index -added 1999-02-01 A.Trocha- */
	case 200:
	    i = intexpr (argptr);
	    namespace = i;
	    break;
/* VIEW 201: configuration index -added 1999-02-01 A.Trocha- */
	case 201:
	    i = intexpr (argptr);
	    config = i;
	    break;
/* VIEW 202: Z-command argument name -- added 1999-03-15 D.Whitten */
	case 202:
	    if (stlen (argptr) >= PATHLEN) {
		ierr = MXSTR;
		return;
	    }
	    stcpy (zargdefname, argptr);
	    return;
	}				/* end switch one parameter VIEWs */
    } else {				/* no parameters VIEWs */
	switch (arg1) {
/* VIEW 21: close all globals */
	case 21:
	    close_all_globals ();
	    return;
/* VIEW 22: clear all v22_aliases */
	case 22:
	    if (v22size)
		free (v22ali);
	    v22ptr = 0L;
	    v22size = 0L;
	    return;
/* VIEW 28: symtab change */
	case 28:			/* get space if not already done */
	    if (apartition == NULL) {
		apartition = calloc ((unsigned) PSIZE + 1, 1);
		asymlen = PSIZE;
		for (i = 0; i < 128; i++)
		    aalphptr[i] = 0L;
	    } {
		char   *x;

		x = apartition;
		apartition = partition;
		partition = x;
	    }
	    ch = asymlen;
	    asymlen = symlen;
	    symlen = ch;
	    for (i = 0; i < 128; i++) {
		ch = aalphptr[i];
		aalphptr[i] = alphptr[i];
		alphptr[i] = ch;
	    }
	    s = &partition[symlen] - 256;	/* pointer to symlen_offset        */
	    argptr = partition;		/* pointer to beg of tmp-storage   */
	    return;
/* VIEW 29: symtab copy */
	case 29:			/* get space if needed */
	    if (apartition == NULL)
		apartition = calloc ((unsigned) (PSIZE + 1), 1);
	    for (i = 0; i <= PSIZE; i++)
		apartition[i] = partition[i];
	    asymlen = symlen;
	    for (i = 0; i < 128; i++)
		aalphptr[i] = alphptr[i];
	    return;

	}
	ierr = ARGER;
	return;
    }
}					/* end view_com() */
/******************************************************************************/
void
view_fun (f, a)				/* process VIEW function */
	int     f;			/* number of arguments */
	char   *a;			/* there are the arguments */
{
    int     i;

    if (standard) {
	ierr = NOSTAND;
	return;
    }					/* non_standard */
    if (f == 1) {
	f = intexpr (a);
	switch (f) {
/* $V(0): reserved for priority */
/* $V(1) returns Device Status word of terminal */
	case 1:
	    lintstr (a, DSW);
	    break;
	case 2:
	    if (curdir[0] != '/') {	/* working directory not known */
		FILE   *pipdes;

		if ((pipdes = popen ("pwd", "r")) != NULL) {
					/* Not necessarily tied to STRLEN*/
		    fgets (curdir, 255, pipdes);
		    i = strlen (curdir);
		    curdir[i - 1] = EOL;	/* LF at end of string!! */
		    pclose (pipdes);
		}
	    }
	    strcpy (a, curdir);
	    a[strlen (a)] = EOL;
	    break;
/* $V(3): global access path */
	case 3:
	    stcpy (a, glopath);
	    break;
/* $V(4): routine DO access path */
	case 4:
	    stcpy (a, rou0path);
	    break;
/* $V(5): routine ZL access path */
	case 5:
	    stcpy (a, rou1path);
	    break;
/* $V(6): %-global access path */
	case 6:
	    stcpy (a, gloplib);
	    break;
/* $V(7): %-routine DO access path */
	case 7:
	    stcpy (a, rou0plib);
	    break;
/* $V(8): %-routine ZL access path */
	case 8:
	    stcpy (a, rou1plib);
	    break;
/* $V(9): OPEN/USE/CLOSE access path */
	case 9:
	    stcpy (a, oucpath); /*djw:all other paths used stcpy not strcpy*/
	    break;
/* $V(10): zallock table file */
	case 10:
	    strcpy (a, zallotab);
	    a[strlen (a)] = EOL;
	    break;
/* $V(11): lock table file */
	case 11:
	    strcpy (a, locktab);
	    a[strlen (a)] = EOL;
	    break;
/* $V(12): routine global protocoll file */
	case 12:
	    stcpy (a, rgafile);
	    break;
/* $V(13): hardcopy file file */
	case 13:
	    strcpy (a, hcpyfile);
	    a[strlen (a)] = EOL;
	    break;
/* $V(14): journalfile */
	case 14:
	    strcpy (a, jourfile);
	    a[strlen (a)] = EOL;
	    break;
/* $V(15): journalflag (-1 0 +1) */
	case 15:
	    intstr (a, jour_flag);
	    break;
/* $V(16): maximum number of error messages */
	case 16:
	    intstr (a, MAXERR - 1);
	    break;
/* $V(17): intrinsic z_commands */
	case 17:
	    stcpy (a, zcommds);
	    break;
/* $V(18): intrinsic z_functions */
	case 18:
	    stcpy (a, zfunctions);
	    break;
/* $V(19): intrinsic special variables */
	case 19:
	    stcpy (a, zsvn);
	    break;
/* $V(20): break service code */
	case 20:
	    stcpy (a, brkaction);
	    break;
/* $V(21) returns size of last global */
	case 21:
	    if (oldfil[inuse][0] != NUL) {
		lseek (olddes[inuse], 0L, 2);
		lintstr (a, (long) tell (olddes[inuse]));
	    } else
		*a = EOL;
	    break;
/* $V(22): number of v22_aliases */
	case 22:
	    i = 0;
	    f = 0;
	    while (f < v22ptr) {
		i++;
		f += UNSIGN (v22ali[f]) + 1;
	    }
	    intstr (a, i);
	    break;
/* $V(23): contents of 'input buffer' */
	case 23:
	    stcpy (a, ug_buf[io]);
	    break;
/* $V(24)/$V(25) number of screen lines */
	case 24:
	case 25:
	    intstr (a, N_LINES);
	    break;
/* $V(26): DO-FOR-XEC stack pointer */
	case 26:
	    intstr (a, nstx);
	    break;
/* $V(27): DO-FOR-XEC stack pointer (copy on error) */
	case 27:
	    intstr (a, nesterr);
	    break;
/* $V(30): number of mumps arguments */
	case 30:
	    intstr (a, m_argc);
	    break;
/* $V(31): environment variables */
	case 31:
	    f = 0;
	    while (m_envp[f] && m_envp[f][0] != NUL)
		f++;
	    intstr (a, f);
	    break;
/* $V(32): maximum size of a loaded routine */
	case 32:
	    lintstr (a, PSIZE0 - 1);
	    break;
/* $V(33): number of routine buffers */
	case 33:
	    intstr (a, (short int) NO_OF_RBUF);
	    break;
/* $V(34): routine buffer size auto adjust */
	case 34:
	    intstr (a, autorsize);
	    break;
/* $V(35): max. number of concurrently open globals */
	case 35:
	    intstr (a, NO_GLOBLS);
	    break;
/* $V(36): max. number of concurrently open devices */
	case 36:
	    intstr (a, MAXDEV);
	    break;
/* $V(37): max. size of local symbol table */
	case 37:
	    lintstr (a, PSIZE);
	    break;
/* $V(38): symtab size autoadjust */
	case 38:
	    intstr (a, autopsize);
	    break;
/* $V(39): maximum size of users special var.table */
	case 39:
	    lintstr (a, UDFSVSIZ);
	    break;
/* $V(40): z_svntab autoadjust */
	case 40:
	    intstr (a, autousize);
	    break;
/* $V(41): maximum size of DO/XEC/FOR/BREAK stack */
	case 41:
	    intstr (a, NESTLEVLS);
	    break;
/* $V(42): maximum expression stack depth */
	case 42:
	    intstr (a, PARDEPTH);
	    break;
/* $V(43): maximum number of patterns */
	case 43:
	    intstr (a, PATDEPTH);
	    break;
/* $V(44): number of characters that make a name unique */
	case 44:
	    f = glvnflag.one[0];
	    if (f == 0)
		f = 255;
	    intstr (a, f);
	    break;
/* $V(45): name case sensitivity flag */
	case 45:
	    f = UNSIGN (glvnflag.one[1]);
	    toggle (f);
	    intstr (a, f);
	    break;
/* $V(46): maximum length of name plus subscripts */
	case 46:
	    f = UNSIGN (glvnflag.one[2]);
	    if (f == 0)
		f = 255;
	    intstr (a, f);
	    break;
/* $V(47): maximum length of a subscript */
	case 47:
	    f = UNSIGN (glvnflag.one[3]);
	    if (f == 0)
		f = 255;
	    intstr (a, f);
	    break;
/* $V(48): single user flag (globals&LOCK) */
	case 48:
	    intstr (a, lonelyflag);
	    break;
/* $V(49): lower case everywhere flag */
	case 49:
	    intstr (a, lowerflag);
	    break;
/* $V(50): direct mode prompt expression */
	case 50:
	    stcpy (a, prompt);
	    break;
/* $V(50): default direct mode prompt string */
	case 51:
	    stcpy (a, defprompt);
	    break;
/* $V(52): G0 input translation table */
	case 52:
	    stcpy0 (a, G0I[io], 257L);
	    a[255] = EOL;
	    break;
/* $V(53): G0 output translation table */
	case 53:
	    stcpy0 (a, G0O[io], 257L);
	    a[255] = EOL;
	    break;
/* $V(54): G1 input translation table */
	case 54:
	    stcpy0 (a, G1I[io], 257L);
	    a[255] = EOL;
	    break;
/* $V(55): G1 output translation table */
	case 55:
	    stcpy0 (a, G1O[io], 257L);
	    a[255] = EOL;
	    break;
/* $V(60): partial pattern match flag */
	case 60:
	    intstr (a, pattrnflag);
	    break;
/* $V(61): partial pattern supplement character */
	case 61:
	    a[0] = pattrnchar;
	    a[1] = EOL;
	    break;
/* $V(62): random: seed number */
	case 62:
	    lintstr (a, random);
	    break;
/* $V(63): random: parameter a */
	case 63:
	    lintstr (a, ran_a);
	    break;
/* $V(64): random: parameter b */
	case 64:
	    lintstr (a, ran_b);
	    break;
/* $V(65): random: parameter c */
	case 65:
	    lintstr (a, ran_c);
	    break;
/* $V(66): SIGTERM handling flag */
	case 66:
	    intstr (a, killerflag);
	    break;
/* $V(67): SIGHUP handling flag */
	case 67:
	    intstr (a, huperflag);
	    break;
/* ... reserved ... */
/* $V(70): ZSORT/ZSYNTAX flag */
	case 70:
	    intstr (a, s_fun_flag);
	    break;
/* $V(71): ZNEXT/ZNAME flag */
	case 71:
	    intstr (a, n_fun_flag);
	    break;
/* $V(72): ZPREVIOUS/ZPIECE flag */
	case 72:
	    intstr (a, p_fun_flag);
	    break;
/* $V(73): ZDATA/ZDATE flag */
	case 73:
	    intstr (a, d_fun_flag);
	    break;
/* ... reserved ... */
/* $V(79): old ZJOB vs. new ZJOB flag */
	case 79:
	    intstr (a, zjobflag);
	    break;
/* $V(80): 7 vs. 8 bit flag */
	case 80:
	    intstr (a, eightbit);
	    break;
/* $V(81): PF1 flag */
	case 81:
	    intstr (a, PF1flag);
	    break;
/* $V(82): order counter */
	case 82:
	    intstr (a, ordercounter);
	    break;
/* $V(83): text in $ZE flag */
	case 83:
	    intstr (a, etxtflag);
	    break;
/* $V(84): path of current routine */
	case 84:			/* look whether we know where the routine came from */
	    for (i = 0; i < NO_OF_RBUF; i++) {
		int     j;

		if (pgms[i][0] == 0) {
		    *a = EOL;
		    return;
		}			/* buffer empty */
		j = 0;
		while (rou_name[j] == pgms[i][j]) {
		    if (rou_name[j++] == EOL) {
			stcpy (a, path[i]);
			i = stlen (a);
			if (i > 0)
			    a[i - 1] = EOL;
			return;
		    }
		}
	    }
	    *a = EOL;
	    break;			/* not found */
/* $V(85): path of last global     */
	case 85:
	    if (oldfil[inuse][0])
		stcpy (a, oldfil[inuse]);
	    else
		*a = EOL;
	    i = 0;
	    while (a[i] != EOL) {
		if (a[i] == '^') {
		    if (i > 0)
			i--;
		    a[i] = EOL;
		    break;
		}
		i++;
	    }
	    break;
/* $V(86): path of current device  */
	case 86:
	    stcpy (a, act_oucpath[io]);
	    break;
/* $V(87): date type definitions */
	case 87:
	    intstr (a, datetype);
	    break;
/* $V(88): date type definitions */
	case 88:
	    intstr (a, timetype);
	    break;
/* $V(89): UNDEF lvn default expression */
	case 89:
	    stcpy (a, lvndefault);
	    break;
/* $V(90): UNDEF gvn default expression */
	case 90:
	    stcpy (a, gvndefault);
	    break;
/* $V(91): missig QUIT expr default expression */
	case 91:
	    stcpy (a, exfdefault);
	    break;
/* $V(92): type mismatch error */
	case 92:
	    intstr (a, typemmflag);
	    break;
/* $V(93): zkey production default rule definition */
	case 93:
	    lintstr (a, v93);
	    break;
/* $V(98): routine extention */
	case 98:
	    stcpy (a, rou_ext);
	    break;
/* $V(99): timer offset */
	case 99:
	    lintstr (a, tzoffset);
	    break;
/* $V(100): exit status of last kill */
	case 100:
	    intstr (a, v100);
	    break;
/* $V(110): local $o/$q data value */
	case 110:
	    stcpy (a, l_o_val);
	    break;
/* $V(111): global $o/$q data value */
	case 111:
	    stcpy (a, g_o_val);
	    break;
/* $V(114): Number of rows in terminal */
	case 114:
	  intstr (a, n_lines);
	  break;
/* $V(115): Number of columns in terminal */
	case 115:
	  intstr (a, n_columns);
	  break;
/* $V(133): remember ZLOAD directory on ZSAVE */
	case 133:
	    intstr (a, zsavestrategy);
	    break;
/* $V(200): namespace index -added 1999-02-01 A.Trocha- */
	case 200:
	    intstr (a, namespace);
	    break;
/* $V(201): configuration index -added 1999-02-01 A.Trocha- */
       	case 201:
	    intstr (a, config);
	    break;

/* $V(202): default varname to pass to user-defined Z commands */
/*                              added 1999-03-15 D.Whitten     */
	case 202:
	    stcpy (a,zargdefname); 
	    break;

	default:
	    ierr = ARGER;
	    return;
	}
	return;
    }
    if (f == 2) {
	char    tmp[256];

	stcpy (tmp, argstck[arg + 1]);
	i = intexpr (argstck[arg + 1]);
	f = intexpr (a);
	if (ierr == MXNUM)
	    return;
	if (f == 16) {
	    if (i <= OK || i >= MAXERR) {
		ierr = ARGER;
		return;
	    } else
		stcpy (a, errmes[i]);
	} else if (f == 22) {		/* return v22_alias entry */
	    if (i) {			/* give one of the names which are aliases */
		f = 0;
		while (f < v22ptr) {
		    i--;
		    if (i == 0) {
			stcpy (a, &v22ali[f + 1]);
			return;
		    }
		    f += UNSIGN (v22ali[f]) + 1;
		}
		a[0] = EOL;
		return;			/* that number had no entry in the table */
	    }
	    if (tstglvn (tmp) == FALSE) {
		ierr = INVREF;
		return;
	    }
	    if (v22ptr) {		/* there are aliases */
		int     k,
		        j;

		i = 0;
		while (i < v22ptr) {
		    k = i + UNSIGN (v22ali[i]) + 1;
		    j = 0;		/* is current reference an alias ??? */
		    while (v22ali[++i] == tmp[j]) {
			if (v22ali[i] == EOL)
			    break;
			j++;
		    }
/* yes, it is, return it */
		    if (v22ali[i] == EOL && tmp[j] == EOL) {
			stcpy (a, &v22ali[i + 1]);
			return;
		    }
		    i = k;
		}
	    }
	    a[0] = EOL;			/* entry was not in the table */
	    return;
	} else if (f == 24) {		/* return screen line */
	    if (i < -N_LINES || i > N_LINES || i == 0)
		*a = EOL;
	    else if (i < 0) {
		stcpy0 (a, (*screen).screena[(unsigned int) (*screen).sclines[-i - 1]], (long) N_COLUMNS);
		a[80] = EOL;
		return;
	    } else {
		stcpy0 (a, (*screen).screenx[(unsigned int) (*screen).sclines[i - 1]], (long) N_COLUMNS);
		a[80] = EOL;
		return;
	    }
	} else if (f == 25) {		/* return screen line with attribute */
	    i--;
	    if (i < 0 || i >= N_LINES)
		*a = EOL;
	    else
		v25 (a, i);
	    return;
	} else if (f == 26) {		/* $V(26) returns DO-FOR-XEC stack pointer */
	    if (i < 1 || i > nstx) {
		ierr = ARGER;
		return;
	    }
	    getraddress (a, i);
	    return;
	}
/* $V(27) returns DO-FOR-XEC stack pointer(error state) */
	else if (f == 27) {
	    if (i < 1 || i > nesterr) {
		ierr = ARGER;
		return;
	    }
	    stcpy (a, callerr[i]);
	    return;
	} else if (f == 30) {		/* $V(30): arguments of mumps */
	    if (i < 1 || i > m_argc) {
		ierr = ARGER;
		return;
	    }
	    strcpy (a, m_argv[i - 1]);
	    a[strlen (a)] = EOL;
	    return;

	/* guard against very long environment name=value entries */
        } else if (f == 31) {/* $V(31): environment variables */
            f = 0;
            while (m_envp[f] && m_envp[f++][0] != NUL) {
                if (f != i)
                    continue;
	        if ((f=strlen(m_envp[i - 1])) > STRLEN) {
		    ierr=MXSTR;
		    return; 
		}
                strcpy (a, m_envp[i - 1]);
                a[f] = EOL;		
                return;
            }
            ierr = ARGER;
            return;
        } else if (f == 93) {/* $V(93): zkey production rule definition */
	    if (i <= 0 || i > NO_V93)
		ierr = ARGER;
	    else
		strcpy (a, v93a[i - 1]);
	    return;
	} else if (f == 113) {		/* $V(113): get termio infos */
	    struct termio tpara;

	    if (i < 1 || i > MAXDEV) {
		ierr = NODEVICE;
		return;
	    }
	    if (devopen[i] == 0) {
		ierr = NOPEN;
		return;
	    }
	    ioctl (fileno (opnfile[i]), TCGETA, &tpara);
	    intstr (a, tpara.c_iflag);
	    i = stlen (a);
	    a[i++] = ':';
	    intstr (&a[i], tpara.c_oflag);
	    i = stlen (a);
	    a[i++] = ':';
	    intstr (&a[i], tpara.c_cflag);
	    i = stlen (a);
	    a[i++] = ':';
	    intstr (&a[i], tpara.c_lflag);
	    return;
	} else {
	    ierr = ARGER;
	    return;
	}
    } else if (f == 3) {
	char    tmp[256];

	stcpy (tmp, argstck[arg + 2]);
	i = intexpr (argstck[arg + 1]);
	f = intexpr (a);
	if (ierr == MXNUM)
	    return;
	if (f == 87) {			/* $V(87): date type definitions */
	    if (i < 0 || i >= NO_DATETYPE) {
		ierr = ARGER;
		return;
	    }
	    f = intexpr (tmp);
	    if (f > 0 && f < 13) {
		stcpy (a, month[i][f - 1]);
		return;
	    }
	    switch (f) {
	    case 13:{
		    stcpy (a, dat1char[i]);
		    return;
		}
	    case 14:{
		    stcpy (a, dat2char[i]);
		    return;
		}
	    case 15:{
		    a[0] = dat3char[i];
		    a[1] = EOL;
		    return;
		}
	    case 16:{
		    a[0] = dat4flag[i] + '0';
		    a[1] = EOL;
		    return;
		}
	    case 17:{
		    a[0] = dat5flag[i] + '0';
		    a[1] = EOL;
		    return;
		}
	    case 18:{
		    lintstr (a, datGRbeg[i] - 672411L);
		    return;
		}
	    }
	} else if (f == 88) {		/* $V(88): time type definitions */
	    if (i < 0 || i >= NO_TIMETYPE) {
		ierr = ARGER;
		return;
	    }
	    f = intexpr (tmp);
	    switch (f) {
	    case 1:{
		    a[0] = tim1char[i];
		    a[1] = EOL;
		    return;
		}
	    case 2:{
		    a[0] = tim2char[i];
		    a[1] = EOL;
		    return;
		}
	    case 3:{
		    a[0] = tim3char[i];
		    a[1] = EOL;
		    return;
		}
	    case 4:{
		    a[0] = tim4flag[i] + '0';
		    a[1] = EOL;
		    return;
		}
	    case 5:{
		    a[0] = tim5flag[i] + '0';
		    a[1] = EOL;
		    return;
		}
	    }
	}
	ierr = ARGER;
	return;
    } else {
	ierr = FUNARG;
	return;
    }
    return;
}					/* end view_fun() */
/******************************************************************************/
void
m_tolower (str)
	char   *str;			/* string to be made lowercase */
{
    int     ch;

    while ((ch = *str) != EOL) {
	ch = *str;
	if (ch <= 'Z' && ch >= 'A') {
	    ch += 32;
	    *str = ch;
	}
	str++;
    }
    return;
}					/* end tolower() */
/******************************************************************************/
short int
newpsize (size)
	long    size;			/* desired size of 'partition' */
{
    char   *newpart;
    char   *anewpart;
    long    dif,
            j;

    if (size == PSIZE)
	return 0;			/* nothing changes */
    if (size <= (PSIZE - symlen + 512))
	return 0;			/* cannot decrease it now */
    if (apartition &&
	size <= (PSIZE - asymlen + 512))
	return 0;			/* cannot decrease it now */

    newpart = calloc ((unsigned) (size + 1), 1);
    if (newpart == NULL)
	return 1;			/* could not allocate stuff */
    if (apartition) {
	anewpart = calloc ((unsigned) (size + 1), 1);
	if (anewpart == NULL) {
	    free (newpart);
	    return 1;
	}				/* no more space */
    }
    dif = argptr - partition + 256;
    if (dif > PSIZE)
	dif = PSIZE;
    stcpy0 (newpart, partition, dif);	/* intermediate results */
    dif = size - PSIZE;
    stcpy0 (&newpart[symlen + dif], &partition[symlen], PSIZE - symlen);
    if (apartition)
	stcpy0 (&anewpart[asymlen + dif], &apartition[asymlen], PSIZE - asymlen);
    for (j = '%'; j <= 'z'; j++) {	/* update alphpointers */
	if (alphptr[j])
	    alphptr[j] += dif;
	if (aalphptr[j])
	    aalphptr[j] += dif;
    }
    PSIZE = size;
    symlen += dif;
    asymlen += dif;
    free (partition);			/* free previously allocated space */
    if (apartition)
	free (apartition);		/* free previously allocated space */
    dif = newpart - partition;
    partition = newpart;
    if (apartition)
	apartition = anewpart;
    s = &partition[symlen] - 256;	/* pointer to symlen_offset        */
    argptr += dif;			/* pointer to beg of tmp-storage   */
    for (j = 0; j <= PARDEPTH; j++)
	if (argstck[j])
	    argstck[j] += dif;

    return 0;
}					/* end newpsize() */
/******************************************************************************/
short int
newusize (size)
	long    size;			/* desired size of svn_table */
{
    char   *newsvn;
    long    dif,
            j;

    if (size <= (UDFSVSIZ - svnlen))
	return 0;			/* cannot decrease it now */
    if (size == UDFSVSIZ)
	return 0;			/* nothing changes */

    newsvn = calloc ((unsigned) (size + 1), 1);
    if (newsvn == NULL)
	return 1;			/* could not allocate stuff */
    stcpy0 (newsvn, svntable, svnlen);	/* intermediate results */
    dif = size - UDFSVSIZ;
    stcpy0 (&newsvn[svnlen + dif], &svntable[svnlen], UDFSVSIZ - svnlen);
    for (j = '%'; j <= 'z'; j++) {	/* update svn_alphpointers */
	if (svnaptr[j])
	    svnaptr[j] += dif;
    }
    UDFSVSIZ = size;
    svnlen += dif;
    free (svntable);			/* free previously allocated space */
    svntable = newsvn;

    return 0;
}					/* end newusize() */
/******************************************************************************/
short int
newrsize (size, nbrbuf)
	long    size;			/* desired size of routinebuffers  */
	long    nbrbuf;			/* desired number of routiebuffers */
{
    char   *newrbuf;
    int     i;
    long    dif;
    unsigned long total;

    if (size <= (rouend - rouptr + 1))
	return 0;			/* making it smaller would be a mistake */

    if (nbrbuf > MAXNO_OF_RBUF)
	nbrbuf = MAXNO_OF_RBUF;

    total = (unsigned) nbrbuf *(unsigned) size;

/* some overflow ??? */
    if ((total / (unsigned) size) != (unsigned) nbrbuf) {
	ierr = ARGER;
	return 1;
    }
    newrbuf = calloc (total, 1);	/* routine buffer pool             */
    while (newrbuf == NULL) {		/* could not allocate stuff...     */
	if (--nbrbuf < 2)
	    return 1;			/* ...so try with less buffers     */
	total = (unsigned) nbrbuf *(unsigned) size;

	newrbuf = calloc (total, 1);
    }
/* clear all routine buffers but one */
    for (i = 0; i < MAXNO_OF_RBUF; i++) {	/* empty routine buffers */
	pgms[i][0] = 0;
	ages[i] = 0L;
    }
/* transfer to new buffer */
    stcpy0 (newrbuf, rouptr, (long) (rouend - rouptr + 1));
    dif = newrbuf - rouptr;
    rouend += dif;
    ends[0] = rouend;
    stcpy (pgms[0], rou_name);
    rouins += dif;
    if (roucur == (buff + (NO_OF_RBUF * PSIZE0 + 1)))
	roucur = newrbuf + (nbrbuf * size + 1);
    else
	roucur += dif;
    rouptr = newrbuf;

    free (buff);			/* free previously allocated space */
    buff = newrbuf;
    NO_OF_RBUF = nbrbuf;
    PSIZE0 = size;
    return 0;
}					/* end newrsize() */
/******************************************************************************/
void
zreplace (a, b, c)
	char   *a,
	       *b,
	       *c;

{
    long int ch,
            f,
            l,
            m,
            n;
    char    d[256];

    if (b[0] == EOL)
	return;				/* 2nd argument was empty */
    l = stlen (c);			/* length of 3rd argument */
    n = 0;
    f = 0;
    for (;;)
    {
	m = 0;
	while ((ch = a[f + m]) == b[m] && ch != EOL)
	    m++;
	if (b[m] == EOL) {
	    if (n + l > STRLEN) {
		ierr = MXSTR;
		return;
	    }
	    stcpy0 (&d[n], c, l);
	    n += l;
	    f += m;
	} else {
	    m = 1;
	    if (n + 1 > STRLEN) {
		ierr = MXSTR;
		return;
	    }
	    d[n++] = a[f++];
	}
	if (a[f] == EOL)
	    break;
    }
    d[n] = EOL;
    stcpy (a, d);
    return;
}					/* end zreplace() */
/******************************************************************************/
short int
tstglvn (a)				/* tests whether 'a' is a proper unsubscripted glvn */
	char   *a;
{
    int     i,
            ch;

    i = 0;
    if (a[0] == '^') {
	while (((ch = a[++i]) >= 'A' && ch <= 'Z') ||
	       (ch >= 'a' && ch <= 'z') ||
	       (ch >= '0' && ch <= '9') ||
	       ((ch == '%' && i == 1) ||
		(standard == 0 &&
		 (((ch == '.' || ch == '/') && i == 1) ||
		  (((ch == '/' && a[i - 1] != '/') ||
		    (ch == '%' && a[i - 1] == '/')) &&
		   (a[1] == '.' || a[1] == '/')))))) ;
	return a[i] == EOL;
    }
    if ((ch = a[i++]) != '%' && (ch < 'A' || ch > 'Z') && (ch < 'a' || ch > 'z'))
	return FALSE;
    while ((ch = a[i++]) != EOL)
	if ((ch < '0' || ch > '9') &&
	    (ch < 'A' || ch > 'Z') &&
	    (ch < 'a' || ch > 'z'))
	    return FALSE;
    return TRUE;
}					/* end tstnam() */
/******************************************************************************/
void
zname (a, b)
	char   *a,
	       *b;

{
    int     i,
            j,
            f,
            n;

    i = 0;
    j = 0;
    f = FALSE;				/* we are in name section (vs.subscr.) */
    n = FALSE;				/* part is numeric (vs.alphabetic) */
    while ((a[i] = b[j++]) != EOL) {
	if (a[i] == '"')
	    a[++i] = '"';
	if (a[i] == DELIM) {
	    if (f) {
		if (n == FALSE)
		    a[i++] = '"';
		if (i >= (STRLEN-2)/*was 253*/) {
		    a[i] = EOL;
		    ierr = MXSTR;
		    return;
		}
		a[i] = ',';
		if ((n = znamenumeric (&b[j])) == FALSE)
		    a[++i] = '"';
	    } else {
		a[i] = '(';
		f = TRUE;
		if ((n = znamenumeric (&b[j])) == FALSE)
		    a[++i] = '"';
	    }
	}
	if (++i >= STRLEN) {
	    a[STRLEN] = EOL;
	    if (b[j] != EOL) {
		ierr = MXSTR;
		return;
	    }
	}
    }
    if (f) {
	if (i > (STRLEN-2) /* was 253 */) {
	    ierr = MXSTR;
	    return;
	}
	if (n == FALSE)
	    a[i++] = '"';
	a[i++] = ')';
	a[i] = EOL;
    }
    return;
}					/* end zname() */
/******************************************************************************/
short int
znamenumeric (str)
	char   *str;			/** boolean function that tests

					 *  whether str is a canonical
					 *  numeric */
{
    register ptr = 0,
            ch;
    register point;

    if (str[0] == '-')
	ptr = 1;
    if (str[ptr] == EOL)
	return FALSE;
    if (str[ptr] == DELIM)
	return FALSE;
    if (str[ptr] == '0')
	return str[1] == EOL || str[1] == DELIM;	/* leading zero */
    point = FALSE;
    while ((ch = str[ptr++]) != EOL && ch != DELIM) {
	if (ch > '9')
	    return FALSE;
	if (ch < '0') {
	    if (ch != '.')
		return FALSE;
	    if (point)
		return FALSE;		/* multiple points */
	    point = TRUE;
	}
    }
    if (point) {
	if ((ch = str[ptr - 2]) == '0')
	    return FALSE;		/* trailing zero */
	if (ch == '.')
	    return FALSE;		/* trailing point */
    }
    return TRUE;
}					/* end of znamenumeric() */
/******************************************************************************/
void
procv22 (key)				/* process v22 translation */
	char   *key;
{
    int     i,
            j,
            k1;
    char    tmp1[256];

    if (*key == EOL || *key == 0)
	return;
    i = 0;
    j = 0;
    while (i < v22ptr) {
	k1 = i + UNSIGN (v22ali[i]) + 1;
/* is current reference an alias ??? */
	j = 0;
	while (v22ali[++i] == key[j]) {
	    if (v22ali[i] == EOL)
		break;
	    j++;
	}
/* yes, it is, so resolve it now! */
	if (v22ali[i] == EOL && (key[j] == EOL || key[j] == DELIM)) {
	    stcpy (tmp1, key);
	    stcpy (key, &v22ali[i + 1]);
	    stcat (key, &tmp1[j]);
	    i = 0;
	    continue;			/* try again, it might be a double alias! */
	}
	i = k1;
    }
    return;
}					/* end of procv22() */
/******************************************************************************/
void
v25 (a, i)
	char   *a;
	int     i;
{
    short   c,
            exc,
            k,
            l,
            p;

    k = 0;
    exc = ~((*screen).screena[(unsigned int) (*screen).sclines[i]][0]);
    for (l = 0; l < N_COLUMNS; l++) {
	p = exc;
	exc = (*screen).screena[(unsigned int) (*screen).sclines[i]][l];
	c = (*screen).screenx[(unsigned int) (*screen).sclines[i]][l];
#ifdef NEVER
/* this may result in a problem, when in a system */
/* different G0O/G1O sets are in use !!!          */
	if (((exc == 1 && (p == 0)) || ((exc == 0) && (p == 1))) &&
	    (G0O[HOME][c] == G1O[HOME][c])
		)
	    exc = p;			/* if char looks same in SI/SO, delay SI/SO */
#endif /* NEVER */
	if (exc != p) {			/* set attribute */
#ifdef SCO
	    p = p & ~04;		/* suppress SGR(3) */
	    if (p & 0200)
		p = p & 0201;		/* no display */
	    if (p & 0100)
		p = p & 0101;		/* inverse */
#endif /* SCO */
	    if ((p & 01) != (exc & 01))
		a[k++] = (exc & 01) ? SO : SI;
	    if ((p & ~01) != (exc & ~01)) {
		a[k++] = ESC;
		a[k++] = '[';
		for (p = 1; p < 8; p++) {
		    if (exc & (1 << p)) {
#ifdef SCO
			if (p == 1) {
			    a[k++] = '1';
			    a[k++] = ';';
			    continue;
			}
#endif /* SCO */
			a[k++] = '1' + p;
			a[k++] = ';';
		    }
		}
		if (a[k - 1] == ';')
		    k--;
		a[k++] = 'm';
	    }
	}
	a[k++] = c;
    }
    if (exc & 01)
	a[k++] = SI;
    a[k] = EOL;
    return;
}					/* end of v25() */
/******************************************************************************/

/* End of $Source: /cvsroot-fuse/gump/FreeM/src/views.c,v $ */
