/***
 * $Source: /cvsroot-fuse/gump/FreeM/src/restore.c,v $
 * $Revision: 1.6 $ $Date: 2000/02/22 17:48:24 $
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
 * global restore
 * 
 */

/* restores mumps globals.
 * needs free disk storages approx. the size of the
 * global to be restored. only data blocks are interpreted.
 * it cannot recover from some errors in data blocks.
 * anyway the global to be restored should be saved for
 * a detailed analysis of the error.
 */

#include "mpsdef0.h"
#include <stdio.h>
/* needed if byte data are to be interpreted as unsigned integer */
#define UNSIGN(A) ((A)&0377)

#define g_EOL 30
#define POINT 28
#define MINUS 26

#define ROOT 0L
	/* length of blocks. status bytes defined as offset to blocklength */
#define DATALIM (BLOCKLEN-11)
#define LLPTR   (BLOCKLEN-10)
#define NRBLK    LLPTR
#define RLPTR   (BLOCKLEN- 6)
#define FREE     RLPTR
#define BTYP    (BLOCKLEN- 3)
#define OFFS    (BLOCKLEN- 2)

#define EMPTY    0
#define FBLK     1
#define POINTER  2
#define BOTTOM   6
#define DATA     8

#define PROTECT 30

short   ierr;				/* immediate error status */

					/* dummies included from mumps.c */
char    zref[256];			/* dummy: $R last global reference */
char    varnam[256];			/* dummy:variable/array/function name */

					/* trace vars for global module */
char    glopath[] =
{EOL};					/* path to access globals        */
char    gloplib[] =
{EOL};					/* path to access %globals   */
short   olddes[NO_GLOBLS];		/* filedescr of open global files */
long    g_ages[NO_GLOBLS];		/* last access of global files */
char    oldfil[NO_GLOBLS][40];		/* names of open global files */
short   usage[NO_GLOBLS];		/* usage count of global files */
short   inuse = 0;			/* file in use */
short   nakoffs = 0;			/* offset to naked reference       */
short   colla = 0;			/* 0=numeric   1=alphabetic colla. */
unsigned long traceblk[TRLIM];		/* trace stack - block numbers */
short   traceadr[TRLIM];		/*             - status        */
short   trx;				/*             - stack pointer */
char    compactkey[512];		/* internal form of key in global.c */
int     ordercounter = 0;		/* VIEW 82: order counter */
long    ordercnt = 1L;			/* repeater for $order/$query */
long    v22ptr = 0L;			/* view 22 aliases pointer         */
union four_fl {
    long unsigned all;
    char    one[4];
} glvnflag;				/* [0] unique name chars          0=no limit */

					/* [1] case sensitivity flag      0=sensitive */
					/* [2] max. name+subscripts       0=no limit */
					/* [3] max. length of a subscript 0=no limit */
short   setop = 0;			/* SET op flag             */
short   zprecise = 9;			/* $ZPRECISION of arithmetic      */
char    rgafile[PATHLEN] = "\201";	/* routine/global access protocol file */
FILE   *rgaccess;			/* dto. filedes */
short   pattrnflag = FALSE;		/* incomplete match flag */
char    pattrnchar = EOL;		/* incomplete match flag supplement */
char    zmc[128] = "\
\000\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\
\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037\177\201";

					/* $ZMC loadable match 'controls'  */
char    zmn[128] = "0123456789\201";	/* $ZMN loadable match 'numerics'  */

					/* $ZMP loadable match 'punctuation' */
char    zmp[128] = " !\042#$%&'()*+,-./:;<=>?@^_`\201";

					/* $ZML loadable match 'lowercase' */
char    zml[128] = "abcdefghijklmnopqrstuvwxyz{|}~\201";

					/* $ZMU loadable match 'uppercase' */
char    zmu[128] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]\201";

int     lonelyflag = FALSE;		/* single user flag */
int     lowerflag = FALSE;		/* lowercase everywhere flag */

char    l_o_val[256] = "\201";		/* VIEW 110: local $o/$q data value */
char    g_o_val[256] = "\201";		/* VIEW 111: global $o/$q data value */

void
main (argc, argv)
	int     argc;			/* arguments count */
	char   *argv[];			/* arguments string */

{
    char    filnam[40];			/* global to be restored     */
    static
    char    savnam[512] = "^/usr/tmp/",	/* intermediate storage      */
            unlnam[40] = "/usr/tmp/^";	/* "unlink" filename         */
    short   fildes;			/* file descriptor to filnam */
    char    block[BLOCKLEN];
    char    key[512];
    char    data[512];
    unsigned long blknbr;
    long    offset;
    short   type;
    long    length;
    long    koffs;

    register i,
            j,
            k,
            ch;

    umask (0);				/* protection bits mask to full rights */
    filnam[0] = '^';
    filnam[1] = 0;

    if (argc > 1) {
	j = 0;
	while (--argc > 0) {
	    j++;
	    if (**(argv + j) == '-') {
		printf ("usage is: %s [^]global\012\015", *argv);
		exit (0);
	    }
	    strcpy (&filnam[1], *(argv + j));
	}
    } else {
	printf ("\012\015%s global ^", *argv);
	scanf ("%s", &filnam[1]);
    }

    j = filnam[1];
    if (j == '.' || j == '/' || j == '^') {
	j = 0;
	while ((filnam[j] = filnam[j + 1]))
	    j++;
    }
    if ((fildes = open (filnam, 0)) == -1) {
	printf ("cannot open file %s\007\012\015", filnam);
	exit (1);
    }
    strcpy (&savnam[10], &filnam[1]);
    koffs = 10 + strlen (&filnam[1]);
    strcpy (&unlnam[10], &filnam[1]);
    unlink (unlnam);			/* kill previous tmp_file */
    blknbr = ROOT;

  again:;

    lseek (fildes, blknbr * BLOCKLEN, 0);
    blknbr++;
    if (read (fildes, block, BLOCKLEN) == 0) {
	strcpy (block, "mv /usr/tmp/\\^");
	strcat (block, &filnam[1]);
	strcat (block, " .");
	system (block);
	exit (0);
    }
    type = block[BTYP];
    if (type != DATA)
	goto again;

    offset = UNSIGN (block[OFFS]) * 256 +
	    UNSIGN (block[OFFS + 1]);
    if (offset > DATALIM) {
	printf ("illegal offset %d\012", offset);
	offset = DATALIM;
    }
    i = 0;
    while (i < offset) {
	length = UNSIGN (block[i++]);
	k = UNSIGN (block[i++]);
	if ((i + length) > offset)
	    break;
	for (j = 0; j < length; j++)
	    key[k++] = block[i++];
	key[k] = g_EOL;
/*----------------------*/
	{
	    long    ch0,
	            i,
	            j,
	            k;

	    j = 0;
	    i = 0;
	    data[j++] = DELIM;
	    k = 1;
	    while ((ch = UNSIGN (key[i++])) != g_EOL) {
		if (k) {
		    k = 0;
		}
		ch0 = (ch >= SP ? (ch >> 1) :	/* 'string' chars */
		       (ch < 20 ? (ch >> 1) + '0' :	/* 0...9          */
			(ch >> 1) + SP));	/* '.' or '-'     */
		if (ch0 == DEL) {
		    if (((ch = UNSIGN (key[i++])) >> 1) == DEL) {
			ch0 += DEL;
			ch = UNSIGN (key[i++]);
		    }
		    ch0 += (ch >> 1);
		}
		data[j++] = ch0;
		if (ch & 01) {
		    data[j++] = DELIM;
		    k = 1;
		}
	    }
	    data[j--] = EOL;
	    if (j == 0)
		data[0] = EOL;
	    else if (data[j] == DELIM)
		data[j] = EOL;
	    if ((koffs + j) > 255)
		goto again;		/* oversize subscript */
	    while (j >= 0) {
		if ((UNSIGN (ch = data[--j]) < SP) && (ch != DELIM))
		    break;
	    }

	    if (j < 0)
		stcpy (&savnam[koffs], data);
	    else
		goto again;		/* illegal subscipt */
/*----------------------*/
	}
	length = UNSIGN (block[i++]);
	k = 0;
	if ((i + length) > offset)
	    break;
	stcpy0 (data, &block[i], length);
	i += length;
	data[length] = EOL;
	global  (0, savnam, data);	/* call original global */

	if (ierr == PROTECT) {
	    printf ("\012cannot open intermediate file in /usr/tmp\012");
	    exit (1);
	}
    }
    if (i != offset)
	printf ("\012wrong offset %d vs. %d\012", offset, i);
    goto again;
}
/******************************************************************************/
void
lintstr (str, integ)			/* converts long integer to string */
	char    str[];
	long    integ;

{
    char    result[11];			/* 32 bit = 10 digits+sign */
    register i = 0;

    if (integ < 0) {
	integ = (-integ);
	*str++ = '-';
    }
    do {
	result[i++] = integ % 10 + '0';
    } while (integ /= 10);
    do {
	*str++ = result[--i];
    } while (i > 0);
    *str = EOL;
    return;
}
/****************************************************************/
void
m_op ()
{
    return;
}					/* dummy call */
int
locking ()
{
    return 0;
}					/* dummy call */
void
symtab ()
{
    return;
}					/* dummy call */
void
add ()
{
    return;
}					/* dummy call */
void
procv22 ()
{
    return;
}					/* dummy call */

/* End of $Source: /cvsroot-fuse/gump/FreeM/src/restore.c,v $ */
