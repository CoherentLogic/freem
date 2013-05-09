/***
 * $Source: /cvsroot-fuse/gump/FreeM/src/gl.c,v $
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
 * global lister
 * 
 */

#include <stdio.h>
#include <time.h>

#define EOL '\201'
#define FALSE   0
#define TRUE    1

/* needed if byte data are to be interpreted as unsigned integer */
#define UNSIGN(A) ((A)&0377)

#define g_EOL 30
#define POINT 28
#define MINUS 26

#define ROOT 1L
	/* length of blocks. status bytes defined as offset to blocklength */
#define BLOCKLEN 1024
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

#define LF      10
#define CR      13
#define SP      ' '
#define NUL      0
#define DEL    127
void    gl ();

void
main (argc, argv)
	int     argc;			/* arguments count */
	char   *argv[];			/* arguments string */

{
    int     args;
    char global[80];
    short   df;				/* data only option */
    short   kf;				/* key only option */
    short   nr;				/* naked reference option */
    register i,
            j,
            k,
            ch;

    df = kf = nr = FALSE;
    k = 0;
    args = argc;
    if (args > 1) {
	j = 0;
	while (--args > 0) {
	    j++;
	    if (argv[j][0] == '-') {
		i = 0;
		while ((ch = argv[j][++i])) {
		    if (ch == 'd') {
			df = TRUE;
			continue;
		    }
		    if (ch == 'k') {
			kf = TRUE;
			continue;
		    }
		    if (ch == 'n') {
			nr = TRUE;
			continue;
		    }
		    fprintf (stderr, "usage is: %s [-k] [-d] [-n] [^]global\n", *argv);
		    exit (0);
		}
		continue;
	    }
	    k++;
	}
    }
    if (k == 0) {
	global[0] = '^';

	printf ("\n\rdisplay global ");
	scanf ("%s", &global[1]);
	k = global[1];

	if (k == '.' || k == '/' || k == '^') {
	    k = 0;
	    while ((global[k] = global[k + 1]))
		k++;
	}
	gl (global, kf, df, nr);

	exit (0);
    }
    args = 0;
    while (++args < argc) {
	global[0] = '^';
	strcpy (&global[1], *(argv + args));
	k = global[1];

	if (k == '-')
	    continue;			/* option */
	if (k == '.' || k == '/' || k == '^') {
	    k = 0;
	    while ((global[k] = global[k + 1]))
		k++;
	}
	gl (global, kf, df, nr);
    }
    exit (0);
}

void
gl (global, kf, df, nr)
	char   *global;			/* name of global */
	short   kf;			/* key flag       */
	short   df;			/* data flag      */
	short   nr;			/* naked ref flag */
{
    short   filedes;			/* file descriptor */
    char    block[BLOCKLEN];		/* block to be read */
    char    key[512];
    char    prevkey[512];		/* previous key */
    char    data[1024];			/* if data has CTRLs it may become */

/* so long                         */
    unsigned long blknbr;
    short   offset;
    short   length;
    short   koffs;
    short   dkf = TRUE;

    short   CtrlFlag;
    short   n,
            k1;
    register i,
            j,
            k,
            ch;

    if ((filedes = open (global, 0)) == -1) {
	printf ("%s: cannot open\012\015", global);

	return;
    }
    if (kf == FALSE && df == FALSE) {
	kf = TRUE;
	df = TRUE;
    } else
	dkf = FALSE;
    blknbr = ROOT;
    prevkey[0] = 0;
    prevkey[1] = 0;
    prevkey[2] = 0;

    for (;;)
    {
	lseek (filedes, blknbr * BLOCKLEN, 0);
	if (read (filedes, block, BLOCKLEN) == 0) {
	    fprintf (stderr, "\015*** something wrong ***\033[K\n\r");
	    exit (0);
	}
	if (block[BTYP] == DATA)
	    goto first;
	i = UNSIGN (block[0]) + 2;
	blknbr = UNSIGN (block[i]) * 65536 +
		UNSIGN (block[i + 1]) * 256 +
		UNSIGN (block[i + 2]);
    }

  again:;

    if (blknbr == 0) {
	close (filedes);
	return;
    }
    lseek (filedes, blknbr * 1024L, 0);
    read (filedes, block, BLOCKLEN);
  first:;
    offset = UNSIGN (block[OFFS]) * 256 +
	    UNSIGN (block[OFFS + 1]);
    blknbr = UNSIGN (block[RLPTR]) * 65536 +
	    UNSIGN (block[RLPTR + 1]) * 256 +
	    UNSIGN (block[RLPTR + 2]);

    i = 0;
    while (i < offset) {
	length = UNSIGN (block[i++]);
	k = koffs = UNSIGN (block[i++]);
	if ((i + length) > offset)
	    break;
	for (j = 0; j < length; j++)
	    key[k++] = block[i++];
	key[k] = g_EOL;
/*----------------------*/
	{
	    short   ch0,
	            i,
	            j,
	            k;

	    j = 0;
	    i = 0;
	    data[j++] = '(';
	    k = 1;
	    while ((ch = UNSIGN (key[i++])) != g_EOL) {
		if (k) {
		    k = 0;
		    if (ch > SP)
			data[j++] = '"';
		}
		ch0 = (ch >= SP ? (ch >> 1) :	/* 'string' chars */
		       (ch < 20 ? (ch >> 1) + '0' :	/* 0...9          */
			(ch >> 1) + SP));	/* '.' or '-'     */
		if (ch0 == DEL) {
		    if ((ch0 = (UNSIGN (key[i++]) >> 1)) == DEL)
			ch0 = (UNSIGN (key[i++]) >> 1) + DEL;
		    ch0 += DEL;
		    data[j] = '<';
		    data[++j] = '0' + ch0 / 100;
		    data[++j] = '0' + (ch0 % 100) / 10;
		    data[++j] = '0' + ch0 % 10;
		    data[++j] = '>';
		} else
		    data[j] = ch0;
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
	    data[j] = ')';
	    if (j == 0)
		data[0] = 0;
	    while (j >= 0) {
		if ((ch = data[--j]) < SP || ch >= DEL)
		    break;
	    }

	    k1 = 0;
	    if (nr) {
		if (prevkey[0]) {
		    n = ch = 0;
		    while (data[n] == prevkey[n]) {
			if (prevkey[n] == '"')
			    ch = !ch;
			if (!ch && k1 == 0 && (prevkey[n] == '('))
			    k1 = n + 1;
			if (!ch && (prevkey[n] == ','))
			    k1 = n + 1;
			n++;
		    }
		    while (prevkey[n]) {
			if (prevkey[n] == '"')
			    ch = !ch;
			if (!ch && (prevkey[n] == ',')) {
			    k1 = 0;
			    break;
			}
			n++;
		    }
		}
		strcpy (prevkey, data);
		if (k1 > 1) {
		    strcpy (&data[1], &data[k1]);
		}
	    }
	    if (j < 0) {
		if (kf) {
		    if (k1) {
			printf ("%c%s", '^', data);
		    } else
			printf ("%s%s", global, data);
		}
		if (dkf && !nr)
		    printf ("=");
		else if (kf)
		    printf ("\n");
	    } else
		fprintf (stderr, "[%d][%d] <illegal subscipt>\n", length, koffs);
/*----------------------*/
	}
	length = UNSIGN (block[i++]);
	stcpy0 (data, &block[i], (long) length);
	data[length] = EOL;
	if (numeric (data)) {
	    data[length] = 0;
	    i += length;
	} else {
	    CtrlFlag = 0;
	    data[0] = '"';
	    k = 1;
	    while (length-- > 0) {
		ch = UNSIGN (block[i++]);
		if ((ch >= SP) && (ch < DEL)) {
		    if (CtrlFlag) {	/* close Bracket after CTRL */
			data[k++] = ')';
			data[k++] = '_';
			data[k++] = '"';
			CtrlFlag = 0;
		    }
		    if ((data[k++] = ch) == '"')
			data[k++] = ch;
		} else {
		    if (((ch >= NUL) && (ch < SP)) || ch == DEL) {
			if (CtrlFlag) {
			    data[k++] = ',';
			} else {
			    if (k > 1) {
				data[k++] = '"';
				data[k++] = '_';
			    } else {
				k = 0;
			    }
			    data[k++] = '$';
			    data[k++] = 'C';
			    data[k++] = '(';
			    CtrlFlag = 1;
			}
			if (ch == DEL) {
			    data[k++] = '1';
			    ch -= 100;
			}
			if (ch >= 10) {
			    data[k++] = ch / 10 + '0';
			    ch = ch % 10;
			}
			data[k++] = ch + '0';
		    } else {
			if (CtrlFlag) {	/* close Bracket after CTRL */
			    data[k++] = ')';
			    data[k++] = '_';
			    data[k++] = '"';
			    CtrlFlag = 0;
			}
			data[k++] = '<';
			if (ch > 99) {
			    data[k++] = '0' + (ch / 100);
			    ch = ch % 100;
			}
			if (ch > 9) {
			    data[k++] = '0' + (ch / 10);
			    ch = ch % 10;
			}
			data[k++] = '0' + ch;
			data[k++] = '>';
		    }
		}
	    }
	    if (CtrlFlag)
		data[k++] = ')';
	    else
		data[k++] = '"';
	    data[k] = 0;
	}
	if (df)
	    printf ("%s\n", data);
    }
    if (i != offset)
	fprintf (stderr, "\nwrong offset %d vs. %d\n", offset, i);
    goto again;
}
int
numeric (str)
	char    str[];			/** boolean function that tests

					 *  whether str is a canonical
					 *  numeric
 */
{
    register ptr = 0,
            ch;
    register point;

    if (str[0] == '-')
	ptr = 1;
    if (str[ptr] == EOL)
	return FALSE;
    if (str[ptr] == '0')
	return str[1] == EOL;		/* leading zero */
    point = FALSE;
    while ((ch = str[ptr++]) != EOL) {
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
}

/* End of $Source: /cvsroot-fuse/gump/FreeM/src/gl.c,v $ */
