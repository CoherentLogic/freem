/***
 * $Source: /cvsroot-fuse/gump/FreeM/src/verify.c,v $
 * $Revision: 1.6 $ $Date: 2000/02/28 18:02:23 $
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
 * global (validate) verify routine
 * 
 */

/* (validate) verify mumps globals.
 * checks mumps globals and reports any problems.
 * it stops if all blocks have been checked or if
 * a maximum number of error has been found.
 * it returns zero if the checked global is ok
 * and a 1 if not. problems are output on stdout
 */

#include "mpsdef.h"
static short int
g_collate ();			/* if 't' follows 's' in MUMPS collating */

void    check ();
void    show ();

/* maximum level (i.e. tree depth) */
/* trace limit in globals module, i.e. trees wo'nt grow to the sky */
#define MAXLEV 10
/* error limit */
#define ERRLIM 32
/* needed if byte data are to be interpreted as unsigned integer */
#define UNSIGN(A) ((A)&0377)

#define MINUS 26
#define POINT 28
#define OMEGA 29
#define g_EOL 30
#define ALPHA 31

#define ROOT 0L
	/* length of blocks. status bytes defined as offset to blocklength */
#define DATALIM (BLOCKLEN-11)
#define LLPTR   (BLOCKLEN-10)
#define NRBLK    LLPTR
#define RLPTR   (BLOCKLEN- 6)
#define FREE     RLPTR
#define BTYP    (BLOCKLEN- 3)
#define OFFS    (BLOCKLEN- 2)

/* length of blockpointers in bytes */
#define PLEN     3

#define EMPTY    0
#define FBLK     1
#define POINTER  2
#define BOTTOM   6
#define DATA     8

#define PROTECT 30

short   filedes;			/* file descriptor */
char    block[MAXLEV][BLOCKLEN];	/* block to be read */
char   *blck;				/* dto. as pointer */
unsigned long llink[MAXLEV];		/* left link pointers */
unsigned long rlink[MAXLEV];		/* right link pointers */
short   offsets[MAXLEV];		/* offsets */
short   type[MAXLEV];			/* block type */
short   level;				/* current level */
unsigned long no_of_blks;		/* number of blocks */
unsigned long freeblks;			/* free blocks list */
char    key[256];			/* current key in pointer block scan */
short   exstat = 0;			/* exit status */
void    showpath ();			/* display path of pointers */

int
main (argc, argv)
	int     argc;			/* arguments count */
	char   *argv[];			/* arguments string */
{
    char    filnam[80];			/* name of global file */
    register j;

    filnam[0] = '^';
    filnam[1] = 0;

    if (argc > 1) {
	j = 0;
	while (--argc > 0) {
	    j++;			/* accept it with or without '^' sign */
	    if (**(argv + j) == '-') {
		printf ("usage is: %s [^]global\012\015", *argv);
		exit (0);
	    }
	    if (**(argv + j) == '^')
		strcpy (filnam, *(argv + j));
	    else
		strcpy (&filnam[1], *(argv + j));
	}
    } else {
	printf ("\n\r%s global ^", *argv);
	scanf ("%s", &filnam[1]);
    }
    if ((filedes = open (filnam, 0)) == -1) {
	printf ("cannot open file %s\007\n\r", filnam);
	exit (0);
    }
    j = 0;
    while (j < MAXLEV) {
	rlink[j] = 0;
	llink[j] = 0;
	j++;
    }
    level = 0;

    check (ROOT);

    j = 1;				/* ignore level zero: there is no rightlink pointer (freeblocks instead) */
    while (j < MAXLEV) {		/* check last right link pointers (all zero!) */
	if (rlink[j] != 0) {
	    printf ("block #%ld right link pointer mismatch 0 vs. %ld\012\015", llink[j], rlink[j]);
	    showpath ();
	}
	j++;
    }
/* check free blocks */
    freeblks = UNSIGN (block[0][FREE]) * 65536 +
	    UNSIGN (block[0][FREE + 1]) * 256 +
	    UNSIGN (block[0][FREE + 2]);

    while (freeblks) {
	unsigned long free;
	int     i;

	if (freeblks > no_of_blks) {
	    printf ("block# %ld (free list) greater than number of blocks (%ld)\012\015", freeblks, no_of_blks);
	    showpath ();
	}
	lseek (filedes, (long) (freeblks) * BLOCKLEN, 0);
	if (read (filedes, block[0], BLOCKLEN) < BLOCKLEN) {
	    printf ("block #%ld is (partially) empty\012\015", freeblks);
	    showpath ();
	    exit (exstat);
	}
	j = UNSIGN (block[0][OFFS]) * 256 +
		UNSIGN (block[0][OFFS + 1]);	/* offset */
	freeblks = UNSIGN (block[0][RLPTR]) * 65536 +
		UNSIGN (block[0][RLPTR + 1]) * 256 +
		UNSIGN (block[0][RLPTR + 1]);
	i = 0;
	while (i < j) {
	    free = UNSIGN (block[0][i++]) * 65536;
	    free += UNSIGN (block[0][i++]) * 256;
	    free += UNSIGN (block[0][i++]);
	    if (free > no_of_blks) {
		printf ("block# %ld (free) greater than number of blocks (%ld)\012\015", free, no_of_blks);
		showpath ();
	    }
	    lseek (filedes, free * BLOCKLEN, 0);
	    read (filedes, block[1], BLOCKLEN);
	    if (block[1][BTYP] != EMPTY) {
		printf ("block #%ld expected block type: EMPTY\012\015", free);
		if (++exstat >= ERRLIM) {
		    printf ("error limit exceeded\012\015");
		    exit (exstat);
		}
	    }
	}
    }
    exit (exstat);
}					/* end main */

void
check (blknbr)
	unsigned long blknbr;		/* check that block */
{
    unsigned long left;			/* current left link pointer */
    unsigned long right;		/* current right link pointer */
    long    i,
            k;

    lseek (filedes, blknbr * BLOCKLEN, 0);
    blck = block[level];
    if (read (filedes, blck, BLOCKLEN) < BLOCKLEN) {
	printf ("block #%ld is (partially) empty\012\015", blknbr);
	showpath ();
    }
    type[level] = blck[BTYP];
    left = UNSIGN (blck[LLPTR]) * 65536 +
	    UNSIGN (blck[LLPTR + 1]) * 256 +
	    UNSIGN (blck[LLPTR + 2]);
    right = UNSIGN (blck[RLPTR]) * 65536 +
	    UNSIGN (blck[RLPTR + 1]) * 256 +
	    UNSIGN (blck[RLPTR + 2]);
    if (blknbr == ROOT)
	no_of_blks = UNSIGN (block[0][NRBLK]) * 65536 +
		UNSIGN (block[0][NRBLK + 1]) * 256 +
		UNSIGN (block[0][NRBLK + 2]);
    else {
	if (blknbr > no_of_blks) {
	    printf ("block# %ld greater than number of blocks (%ld)\012\015", blknbr, no_of_blks);
	    showpath ();
	}
    }
    offsets[level] =
	    UNSIGN (blck[OFFS]) * 256 +
	    UNSIGN (blck[OFFS + 1]);
    if (rlink[level] != 0L && rlink[level] != blknbr) {
	printf ("block #%ld right link pointer mismatch %ld vs. %ld\012\015", llink[level], blknbr, rlink[level]);
	showpath ();
    }
    if (llink[level] != 0L && left != llink[level]) {
	printf ("block #%ld left link pointer mismatch %ld vs. %ld\012\015", blknbr, left, llink[level]);
	showpath ();
    }
    rlink[level] = right;
    llink[level] = blknbr;
    if (blknbr != ROOT) {
	k = UNSIGN (blck[0]);
	i = 0;
	while (i < k) {
	    if (blck[i + 2] != key[i]) {
		printf ("block #%ld first key mismatch to pointer block(%ld)\012\015", blknbr, llink[level - 1]);
		showpath ();
		break;
	    }
	    i++;
	}
    }
    switch (type[level]) {
    case EMPTY:
	printf ("block #%ld unexpected block type: EMPTY\012\015", blknbr);
	showpath ();
	break;
    case FBLK:
	printf ("block #%ld unexpected block type: FBLK\012\015", blknbr);
	showpath ();
	break;
    case POINTER:
    case BOTTOM:
/*******************************/
/* scan pointer block */
	{
	    register long i;
	    register long k;
	    short   j,
	            len;
	    char    key1[256];

	    key1[0] = g_EOL;
	    i = 0;
	    while (i < offsets[level]) {
		j = i++;		/* save adress of current entry */
		if ((len = UNSIGN (blck[j]) + (k = UNSIGN (blck[i++]))) > 255) {
		    printf ("block #%ld key too long\012\015", blknbr);
		    showpath ();
		} else {
		    if (len == 0 && j) {
			printf ("block #%ld empty key\012\015", blknbr);
			showpath ();
		    }
		    while (k < len)
			key[k++] = blck[i++];
		    key[k] = g_EOL;
		    if (key_check (key)) {
			printf ("block #%ld illegal key\012\015", blknbr);
			showpath ();
		    }
		    if (g_collate (key1, key) == 0) {
			printf ("block #%ld collation mismatch\012\015", blknbr);
			show (key1);
			show (key);
			showpath ();
		    }
		    stcpy0 (key1, key, k + 1);
		    level++;
		    check ((long) (UNSIGN (blck[i]) * 65536 +
				   UNSIGN (blck[i + 1]) * 256 +
				   UNSIGN (blck[i + 2])));
		    blck = block[--level];
		}
		i += PLEN;
		if (i > DATALIM) {
		    printf ("block #%ld pointer in status bytes\012\015", blknbr);
		    showpath ();
		}
	    }
	    if (i > offsets[level]) {
		printf ("block #%ld offset mismatch %ld vs. %ld\012\015", blknbr, i, offsets[level]);
		showpath ();
	    }
	}
/*******************************/
	break;
    case DATA:
/*******************************/
/* scan data block */
	{
	    register long i;
	    register long k;
	    short   len;
	    char    key0[256];
	    char    key1[256];

	    if (type[level - 1] != BOTTOM) {
		printf ("block #%ld unexpected block type: DATA\012\015", blknbr);
		showpath ();
	    }
	    key1[0] = g_EOL;
	    i = 0;
	    while (i < offsets[level]) {
		len = UNSIGN (blck[i++]);
		len += (k = UNSIGN (blck[i++]));
		if (len > 255) {
		    printf ("block #%ld key too long\012\015", blknbr);
		    showpath ();
		    i += len - k;
		} else {
		    if (len == 0 && i > 2) {
			printf ("block #%ld empty key\012\015", blknbr);
			showpath ();
		    }
		    while (k < len)
			key0[k++] = blck[i++];
		    key0[k] = g_EOL;
		    if (key_check (key0)) {
			printf ("block #%ld illegal key\012\015", blknbr);
			showpath ();
		    }
		    if (g_collate (key1, key0) == 0) {
			printf ("block #%ld collation mismatch\012\015", blknbr);
			show (key1);
			show (key0);
			showpath ();
		    }
		    stcpy0 (key1, key0, k + 1);
		}
		k = i + 1;
		len = UNSIGN (blck[i]);
		i += UNSIGN (blck[i]);
		i++;			/* skip data */
#ifdef NEVER
		while (k < i)
		    if (blck[k++] & ~0177) {
			printf ("block #%ld illegal character in data string\012\015", blknbr);
			showpath ();
			break;
		    }
#endif /* NEVER */
		if (i > DATALIM) {
		    printf ("block #%ld data in status bytes\012\015", blknbr);
		    showpath ();
		}
	    }
	    if (i > offsets[level]) {
		printf ("block #%ld offset mismatch %ld vs. %ld\012\015", blknbr, i, offsets[level]);
		showpath ();
	    }
	}
/*******************************/
	break;
    default:
	printf ("block #%ld illegal type %ld\012\015", blknbr, type[level]);
	showpath ();
    }
    return;
}					/* end check */
void
showpath ()
{					/* display path of pointers */
    int     i;

    if (level > 1)
	for (i = 0; i < level; i++)
	    printf ("  path level(%ld)=%ld\012\015", i, llink[i]);
    if (++exstat >= ERRLIM) {
	printf ("error limit exceeded\012\015");
	exit (exstat);
    }
    return;
}
/******************************************************************************/
int
key_check (key)				/* checks a global key in compressed form */
	char   *key;
{
    short   ch,
            typ = 0;

    while ((ch = UNSIGN (*key++)) != g_EOL) {
	if (ch == (DEL << 1)) {
	    if ((ch = UNSIGN (*key++)) == (DEL << 1))
		key++;
	    ch = SP;
	}
	if (ch >= SP) {
	    if (typ == 2)
		return 1;
	    typ = 1;
	}
/* alphabetics */
	else {
	    if (typ == 1)
		return 1;
	    typ = 2;			/* numerics '.' '-' */
	    if (ch >= 20 && ch != POINT && ch != MINUS)
		return 1;		/* illegal character */
	}
	if (ch & 01)
	    typ = 0;			/* comma between two indices */
    }
    return 0;
}					/* end key_check */
/******************************************************************************/
static short int
g_collate (s, t)			/* if 't' follows 's' in MUMPS collating */
	char    s[];			/* sequence a TRUE is returned           */
	char    t[];

{
    register chs = *s;
    register cht = *t;
    register tx = 0;
    register sx;
    short   dif;

/* the empty one is the leader! */
    if (chs == g_EOL) {
	if (cht == g_EOL)
	    return 2;
	return 1;
    }
    if (cht == g_EOL)
	return FALSE;

    while (cht == s[tx]) {
	if (cht == g_EOL)
	    return 0;
	cht = t[++tx];
    }					/* (s==t) */
    chs = s[tx];
    if (chs == OMEGA)
	return 0;
    if (chs == ALPHA)
	return cht != g_EOL;
    if (chs == g_EOL && t[tx - 1] & 01)
	return 1;
    if (cht == g_EOL && s[tx - 1] & 01)
	return 0;

/* vade retro usque ad comma */
    if (tx > 0) {
	tx--;
	while ((t[tx] & 01) == 0)
	    if (--tx < 0)
		break;
	tx++;
    }
    chs = s[tx];
    cht = t[tx];
    if (UNSIGN (chs) <= POINT) {	/* then come numerics */
	if (UNSIGN (cht) > POINT)
	    return UNSIGN (cht) != g_EOL;
/* both are numeric! now compare numeric values */
/*****g_comp()*********************************************************/
	if (chs == MINUS) {
	    if (cht != MINUS)
		return 1;
	} else {
	    if (cht == MINUS)
		return 0;
	}
	if (chs == 1 && cht == POINT)
	    return 1;
	if (cht == 1 && chs == POINT)
	    return 0;
	dif = sx = tx;
	while (s[sx] != POINT) {
	    if (s[sx++] & 01)
		break;
	}
	while (t[tx] != POINT) {
	    if (t[tx++] & 01)
		break;
	}
	if (tx > sx)
	    return (cht != MINUS);
	if (tx < sx)
	    return (cht == MINUS);
	tx = dif;
	while ((cht >> 1) == (chs >> 1)) {
	    if (cht & 01)
		return t[dif] == MINUS;
	    if (chs & 01)
		return t[dif] != MINUS;
	    chs = s[++tx];
	    cht = t[tx];
	}
	return (((cht >> 1) > (chs >> 1)) == (t[dif] != MINUS))
		&& (t[tx] != s[tx]);
/**********************************************************************/
    }
    if (UNSIGN (cht) <= POINT)
	return 0;
    while ((dif = (UNSIGN (cht) >> 1) - (UNSIGN (chs) >> 1)) == 0) {	/* ASCII collating */
	if ((cht & 01) && ((chs & 01) == 0))
	    return 0;
	if ((chs & 01) && ((cht & 01) == 0))
	    return 1;
	chs = s[++tx];
	cht = t[tx];
    }
    if (chs == g_EOL)
	return 1;
    if (cht == g_EOL)
	return 0;
    return dif > 0;
}					/* end g_collate */
/******************************************************************************/
void
show (key)
	char    key[];			/* key in internal format to be expanded and shown */
{
    int     k,
            ch,
            i,
            j;
    char    data[256];

    k = 0;
    i = 0;
    j = 0;
    while ((ch = UNSIGN (key[i++])) != g_EOL) {
	if (k) {
	    k = 0;
	    if (ch > ' ')
		data[j++] = '"';
	}
	data[j] = (ch > SP ? (ch >> 1) : (ch < 20 ? (ch >> 1) + '0' : (ch >> 1) + ' '));
	if (data[j++] == '"')
	    data[j++] = '"';
	if (ch & 01) {
	    if (ch > SP)
		data[j++] = '"';
	    data[j++] = ',';
	    k = 1;
	}
    }
    data[j--] = 0;
    printf ("(%s);", data);
    return;
}					/* end show() */
/******************************************************************************/

/* End of $Source: /cvsroot-fuse/gump/FreeM/src/verify.c,v $ */
