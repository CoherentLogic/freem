/***
 * $Source: /cvsroot-fuse/gump/FreeM/src/locks.c,v $
 * $Revision: 1.7 $ $Date: 2000/02/22 17:48:24 $
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
 * program to display the mumps 'locktab'
 * 
 */

#include "mpsdef0.h"

#ifndef FREEBSD
#define EOF -1
#endif

#include <stdio.h>
#ifdef SYSFIVE
#include <fcntl.h>
#endif /* SYSFIVE */

/* needed if byte data are to be interpreted as unsigned integer */
#define UNSIGN(A) ((A)&0377)
void    unlock ();
void    zname ();
short int znamenumeric ();

/* 01/18/99 rlf Apparently, tell disappeared with libc-6 */
#ifdef LINUX_GLIBC
long int
tell (int fd)
{
    return lseek (fd, 0, SEEK_CUR);
}
#endif /* LINUX_GLIBC */

void
main (argc, argv)
	int     argc;			/* arguments count     */
	char  **argv;			/* arguments string    */

{
    static char locktab[80] = "/usr/tmp/locktab";	/* file with LOCKs */
    static long rempid = 0L;		/* remove entry with rem PID */
    short   ltab;			/* file descr. for locktab */
    int     pid;
    short   type;
    char    ch;
    char    line[300],
            varnam[300];
    int     i,
            j,
            n;

    if (argc > 1) {
	j = 0;
	while (--argc > 0) {
	    j++;
	    if (argv[j][0] == '-') {
		if (rempid) {
		    fprintf (stderr, "usage is: %s [-pid] [lockfile]\n", *argv);
		    exit (0);
		}
		n = 0;
		rempid = 0L;
		while ((ch = argv[j][++n])) {
		    if (ch < '0' || ch > '9') {
			fprintf (stderr, "usage is: %s [-pid] [lockfile]\n", *argv);
			exit (0);
		    }
		    rempid = rempid * 10 + ch - '0';
		}
		continue;
	    }
	    strcpy (locktab, *(argv + j));
	}
    }
    if (rempid)
	unlock (locktab, rempid);
    while ((ltab = open (locktab, 0)) == -1) {
	printf ("cannot open '%s'\n", locktab);
	exit (0);
    }

    lseek (ltab, 0L, 0);
    for (;;)
    {
	read (ltab, line, 3);
	pid = UNSIGN (line[0]) * 256 + UNSIGN (line[1]);
	if (pid == 0)
	    break;
	type = line[2];
	i = 0;
	do {
	    read (ltab, &ch, 1);
	    if (ch == EOF)
		goto done;
	    varnam[i++] = ch;
	} while (ch != EOL);
	zname (line, varnam);
	printf ("%d\t%s %s\n", pid, type == 'D' ? "ZA" : "L ", line);
    }
  done:;
    close (ltab);
    exit (0);
}
void
unlock (locktab, pid)			/* unLOCK all entries of pid */
	char   *locktab;		/* locktable */
	long    pid;			/* process ID */

{
    short   ltab;			/* file descr. for locktab */
    int     cpid;
    char    entry[256];
    long int r_pos;			/* position to read  */
    long int w_pos;			/* position to write */
    int     i,
            j;

/*      open locktab, quit if nothing to be done */
    if ((ltab = open (locktab, 2)) == -1)
	return;

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
	if (cpid != pid) {
	    if (j) {
		r_pos = tell (ltab);
		lseek (ltab, w_pos, 0);
		write (ltab, entry, (unsigned) i);
		lseek (ltab, r_pos, 0);
	    }
	    w_pos += i;
	} else
	    j++;
    }
    locking (ltab, 0, 0L);
    close (ltab);
    return;
}					/* end lock() */
#ifdef SYSFIVE
/******************************************************************************/
/* under XENIX the system has an intrinsic function 'locking'.                 */
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
#endif /* SYSFIVE */
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
	if (++i >= 255) {
	    a[255] = EOL;
	}
    }
    if (f) {
	if (n == FALSE)
	    a[i++] = '"';
	a[i++] = ')';
	a[i] = EOL;
    }
    a[i] = 0;
    return;
}					/* end zname() */
/******************************************************************************/
short int
znamenumeric (str)
	char   *str;			/** boolean function that tests

					 *  whether str is a canonical
					 * numeric */
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

/* End of $Source: /cvsroot-fuse/gump/FreeM/src/locks.c,v $ */
