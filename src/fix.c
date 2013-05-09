/***
 * $Source: /cvsroot-fuse/gump/FreeM/src/fix.c,v $
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
 * display mumps global blocks on the CRT
 *
 */

#include <stdio.h>

/* needed if byte data are to be interpreted as unsigned integer */
#define UNSIGN(A) ((A)&0377)

#define g_EOL 30
#define POINT 28
#define MINUS 26

#define ROOT 0L
	/* length of blocks. status bytes defined as offset to blocklength */
#define BLOCKLEN 1024
#define DATALIM (BLOCKLEN-11)
#define LLPTR   (BLOCKLEN-10)
#define NRBLK    LLPTR
#define RLPTR   (BLOCKLEN- 6)
#define FREE     RLPTR
#define BTYP    (BLOCKLEN- 3)
#define OFFS    (BLOCKLEN- 2)

#define PLEN     3

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

void
main (argc, argv)
	int     argc;			/* arguments count */
	char   *argv[];			/* arguments string */

{
    static char ASCII[] = "NULSOHSTXETXEOTENQACKBELBS TABLF VT FF CR SO SI DLEDC1DC2DC3DC4NAKSYNETBCANEM SUBESCFS GS RS US ";
    char    filnam[40];
    short   filedes;
    char    block[BLOCKLEN];
    char    key[512];
    char    data[1024];
    long    blknbr;
    short   offset;
    short   type;
    unsigned long pointer;
    short   length;
    short   koffs;

    register i,
            j,
            k,
            ch;

    filnam[0] = '^';
    filnam[1] = 0;

    if (argc > 1) {
	j = 0;
	while (--argc > 0) {
	    j++;			/* accept it with or without '^' sign */
	    if (**(argv + j) == '-') {
		fprintf (stderr, "usage is: %s [^]global\012\015", *argv);
		exit (0);
	    }
	    if (**(argv + j) == '^')
		strcpy (filnam, *(argv + j));
	    else
		strcpy (&filnam[1], *(argv + j));
	}
    } else {
	printf ("\012\015display global ^");
	scanf ("%s", &filnam[1]);
    }
    if ((filedes = open (filnam, 0)) == -1) {
	printf ("cannot open file %s\007\012\015", filnam);
	exit (0);
    }
  again:;

    printf ("\012\015display block #");
    scanf ("%ld", &blknbr);
    if (blknbr < 0) {
	printf ("\012\015*** done ***\012\015\033[r\033[24H");
	exit (0);
    }
    lseek (filedes, blknbr * 1024L, 0);
    if (read (filedes, block, BLOCKLEN) == 0) {
	printf ("block #%ld does not exist\007\012\015-1 will terminate", blknbr);
	goto again;
    }
    printf ("\033[r\033[2Jglobal\033[;20H%s\033[;40Hblock\033[;60H%ld",
	    filnam,
	    blknbr);
    printf ("\033[2Hblock type\033[2;20H");
    type = block[BTYP];
    switch (type) {
    case DATA:
	printf ("DATA");
	break;
    case POINTER:
	printf ("POINTER");
	break;
    case BOTTOM:
	printf ("BOTTOM POINTER");
	break;
    case EMPTY:
	printf ("EMPTY");
	break;
    case FBLK:
	printf ("FBLK");
	break;
    default:
	printf ("ILLEGAL TYPE");
	type = DATA;
    }
    if (blknbr == ROOT)
	printf (", ROOT");

    offset = UNSIGN (block[OFFS]) * 256 +
	    UNSIGN (block[OFFS + 1]);
    printf ("\033[2;40Hoffset\033[2;60H%d", offset);
    if (offset > DATALIM || (type == FBLK && offset % PLEN)) {
	printf (" ???");
	offset = DATALIM;
    }
    pointer = UNSIGN (block[LLPTR]) * 65536 +
	    UNSIGN (block[LLPTR + 1]) * 256 +
	    UNSIGN (block[LLPTR + 2]);
    printf ("\033[3H%s\033[3;20H%d",
	    blknbr != ROOT ? "LEFT LINK POINTER" : "NR_OF_BLKS",
	    pointer);

    pointer = UNSIGN (block[RLPTR]) * 65536 +
	    UNSIGN (block[RLPTR + 1]) * 256 +
	    UNSIGN (block[RLPTR + 2]);
    printf ("\033[3;40H%s\033[3;60H%d",
	    blknbr != ROOT ? "RIGHT LINK POINTER" : "FREE",
	    pointer);

    printf ("\033[4;24r\033[4H");	/* define scrolling area */

    if (type == FBLK) {
	i = 0;
	while (i < offset) {
	    k = UNSIGN (block[i]) * 65536 +
		    UNSIGN (block[i + 1]) * 256 +
		    UNSIGN (block[i + 2]);
	    i += PLEN;
	    printf ("%8d", k);
	}
	goto again;
    }
    i = 0;
    while (i < offset) {
	printf ("\012\015%3d", i);
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
	            k,
	            typ;

	    j = 0;
	    i = 0;
	    data[j++] = '(';
	    k = 1;
	    while ((ch = UNSIGN (key[i++])) != g_EOL) {
		if (k) {
		    k = 0;
		    if ((typ = (ch > SP)))
			data[j++] = '"';
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
		    if (typ)
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
	    if (j < 0)
		printf ("[%d][%d] %s ", length, koffs, data);
	    else
		printf ("[%d][%d] <illegal subscipt>", length, koffs, data);
/*----------------------*/
	}
	if (type == DATA) {
	    length = UNSIGN (block[i++]);
	    k = 0;
	    if ((i + length) > offset)
		break;
	    while (length-- > 0) {
		ch = UNSIGN (block[i++]);
		if ((ch >= SP) && (ch < DEL))
		    data[k++] = ch;
		else {
		    data[k++] = '<';
		    if ((ch >= NUL) && (ch < SP)) {
			ch = ch * 3;
			data[k++] = ASCII[ch++];
			data[k++] = ASCII[ch++];
			if ((data[k++] = ASCII[ch++]) == SP)
			    k--;
		    } else if (ch == DEL) {
			data[k++] = 'D';
			data[k++] = 'E';
			data[k++] = 'L';
		    } else {
			if (ch > 99) {
			    data[k++] = '0' + (ch / 100);
			    ch = ch % 100;
			}
			if (ch > 9) {
			    data[k++] = '0' + (ch / 10);
			    ch = ch % 10;
			}
			data[k++] = '0' + ch;
		    }
		    data[k++] = '>';
		}
	    }
	    data[k] = 0;
	    printf ("= %s", data);
	} else {
	    pointer = UNSIGN (block[i]) * 65536 +
		    UNSIGN (block[i + 1]) * 256 +
		    UNSIGN (block[i + 2]);
	    i += PLEN;
	    printf ("-> %d", pointer);
	}
    }
    if (i != offset)
	printf ("\012\015wrong offset %d vs. %d\012\015", offset, i);
    goto again;
}

/* End of $Source: /cvsroot-fuse/gump/FreeM/src/fix.c,v $ */
