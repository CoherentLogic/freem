/***
 * $Source: /cvsroot-fuse/gump/FreeM/src/global.c,v $
 * $Revision: 1.7 $ $Date: 2000/02/28 18:02:23 $
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
 * mumps global variable handler
 * 
 */

/* GLOBAL handling */

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
char   *strcpy ();
unsigned int sleep ();

#include "mpsdef.h"

static void b_free ();
static void splitp ();
static void update ();
static void insert ();
static void scanpblk ();
static void scandblk ();
static void getnewblk ();
static short int g_collate ();
static short int g_numeric ();
static void panic ();

#ifndef FREEBSD
long int lseek ();
#endif

#define ROOT 0L

/* end of line symbol in global module is 30, which is a code not */
/* otherwise used in subscripts                                   */
#define g_EOL 30

#define EOL1 EOL

/* numerics (('.'<<1)&037)==28 ; (('-'<<1)&037)==26; */
#define POINT 28
#define MINUS 26

/* ALPHA and OMEGA are dummy subscripts in $order processing */
/* ALPHA sorts before all other subscripts                   */
/* OMEGA sorts after all other subscripts                    */
/* e.g. ("abc") -> "abc",OMEGA ; ("abc","") -> "abc",ALPHA   */
#define OMEGA 29
#define ALPHA 31

	/* length of blocks. status bytes defined as offset to blocklength */
/*      BLOCKLEN 1024           is defined in mpsdef0 include file */
#define DATALIM (BLOCKLEN-11)
#define LLPTR   (BLOCKLEN-10)
#define NRBLK    LLPTR
#define COLLA   (BLOCKLEN- 7)
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

long    time ();

void global (action, key, data)		/* globals management */
	short   action;			/* 0 = store/create 1=retrieve  */

					/* 2 = kill_sym     3=$data     */
					/*                  5=$order    */
					/*                  7=$query    */
					/*                              */
					/* 14=killone      13=getnext   */
					/* 16=merge_sym    17=$zdata    */

	char   *key,			/* gvn as ASCII-string */
	       *data;

	/* returns      OK      action fulfilled        */
	/* (ierr)       UNDEF   missing in action       */
	/*              NAKED   illegal naked reference */
	/*              SBSCR   illegal subscript       */
	/*              DBDGD   data base degradation   */

/* the data are organized in a B* tree structure on external storage.
 * for a description of the principles of the algorithms see
 * =>   donald e.knuth "the art of computer programming" vol.3 p.478
 * This tree structure guarantees fast disc access and is the
 * 'canonical way' to implement mumps globals.
 * 
 * each mumps global occupies a separate unix file in the 'current'
 * directory. %_globals are unix files in the directory '/usr/lib/mumps'.
 * the unix file names are the same as the corresponding mumps global
 * names; i.e. beginning with an '^'.  However it is possible to access
 * globals in other directories if the path name is specified.
 * E.g. "S ^/usr/mumps/test=1" does "S ^test=1 ; in directory /usr/mumps"
 * if mumps is started with the -s 'standard' switch, it is not possible
 * to specify a directory. There is a syntacitc ambiguity: the '/' character
 * in the directory name is in conflict with the '/' divide operator. use
 * brackets to make things clear.
 * 
 * ^/usr/mumps/test/2              ; '/2' is part of the name
 * (^/usr/mumps/test)/2            ; ambiguity resolved
 * ^test/2                         ; '/2' is a divide operation
 * ^/usr/mumps/test("ok")/2        ; '/2' is a divide
 * 
 * to prevent jobs from messing the globals up, access is regulated
 * with the 'locking' mechanism. (that is different from mumps LOCKs)
 * 
 * data are organized in blocks of 1024 bytes (BLOCKLEN) with the following
 * layout:
 * byte    0 - 1013 'stuff'                                  0...DATALIM
 * organization is:
 * length of key (minus offset into previous key)
 * offset into previous key
 * key (without EOL character)
 * length of data               or two bytes as pointer
 * data(without EOL character)     in pointer blocks
 * 
 * byte 1014 - 1016 leftlink pointer                             LLPTR
 * in root block: number of blocks              NRBLK
 * byte 1017        <reserved>
 * byte 1017        in root block: type of collating sequence    COLLA
 * LSB=0: numeric(standard) LSB=1: alphabetic
 * byte 1018 - 1020 rightlink pointer                            RLPTR
 * in root block: number of free blocks list    FREE
 * byte 1021        block type                                   BTYP
 * (2=POINTER,6=BOTTOM LEVEL POINTER,8=DATA)
 * byte 1022 - 1023 offset                                       OFFS
 * (pointer to unused part of 'stuff')
 * 
 * the file is *not* closed on return. since access is regulated by the
 * locking mechanism, that will not spell trouble.
 */

{
/* these must be static variables */

    static short filedes;		/* filedescr for global access */
    static char filnam[256];		/* name of global/unix file */

/* the following vars may be */
/* static or dynamic */

    static unsigned long blknbr;	/* block number */
    static unsigned long oldblk;
    static unsigned long newblk;
    static unsigned long other;
    static long j1;
    static long limit;
    static short typ;			/* block type */
    static long keyl,			/* length of compacted key */
            datal,			/* length of data */
            olddatal,
            offset,
            found,
            addr,			/* address of key in 'block' */
            needed,			/* new bytes needed to ins. stuff */
            ret_to,			/* return code */
            kill_again;
    static char key1[256];
    static char tmp1[256];		/* intermediate storage for op= */
    static char block[BLOCKLEN];
    static int getnflag;		/* flag 1=$ZO-variable 0=$Q-function */
    static int tryfast;			/* try fast access if get_sym on    */

/*                  previous global */
    struct stat dinf;			/* get modification date */

    long    savj,
            savch;			/* saved j and ch for multiple pathes */
    register long int i,
            j,
            k,
            ch;
    long    pathscan;			/* flag for repeated scan of pathlist setting an undef global */

    if (glvnflag.all			/* process optional limitations */
	&& key[0] >= '%' && key[0] <= 'z') {
	if ((i = glvnflag.one[0])) {	/* number of significant chars */
	    j = 0;
	    while ((k = key[j]) != DELIM && k != EOL) {
		if (j >= i) {
		    while ((k = key[++j]) != DELIM && k != EOL) ;
		    stcpy (&key[i], &key[j]);
		    break;
		}
		j++;
	    }
	}
	if (glvnflag.one[1]) {		/* upper/lower sensitivity */
	    j = 0;
	    while ((k = key[j]) != DELIM && k != EOL) {
		if (k >= 'a' && k <= 'z')
		    key[j] = k - 32;
		j++;
	    }
	}
	if ((i = glvnflag.one[2])) {
	    if (stlen (key) > i) {
		ierr = MXSTR;
		return;
	    }				/* key length limit */
	}
	if ((i = glvnflag.one[3])) {	/* subscript length limit */
	    j = 0;
	    while ((k = key[j++]) != DELIM && k != EOL) ;
	    if (k == DELIM) {
		k = 0;
		for (;;)
		{
		    k = key[j++];
		    if (k == DELIM || k == EOL) {
			if (k > i) {
			    ierr = MXSTR;
			    return;
			}
			k = 0;
		    }
		    if (k == EOL)
			break;
		    k++;
		}
	    }
	}
    }
    if (action == getnext) {
	getnflag = TRUE;
	varnam[0] = EOL;
	if (zref[0] == EOL) {
	    ierr = UNDEF;
	    data[0] = EOL;
	    return;
	}
	stcpy (key, zref);
	action = query;
	ordercnt = 1L;
    } else {
	getnflag = FALSE;

/* naked reference section */

	if (key[1] == DELIM) {		/* resolve naked reference */
	    while (--nakoffs >= 0) {	/* naked reference pointer */
		if (zref[nakoffs] == DELIM)
		    break;
	    }
	    if ((i = ++nakoffs) == 0) {	/* illegal naked reference */
		data[0] = EOL1;
		ierr = NAKED;
		return;
	    }
	    j = 2;
	    while ((zref[i] = key[j++]) != EOL) {
		if ((++i) >= STRLEN) {
		    zref[STRLEN] = EOL;
		    ierr = MXSTR;
		    return;
		}
	    }
	    nakoffs = stcpy (key, zref);
	} else
	    nakoffs = stcpy (zref, key);	/* save reference */
    }

    if (v22ptr) {
	procv22 (key);
	if (key[0] != '^') {
	    char    losav[256];

	    stcpy (losav, l_o_val);
	    symtab (action, key, data);
	    stcpy (g_o_val, l_o_val);
	    stcpy (l_o_val, losav);
	    return;
	}
    }
/* construct full UNIX filename */
    savj = 0;
    savch = ch = EOL;
    pathscan = TRUE;
  nextpath:
    k = 0;
    j = savj;
    if (key[1] == '%') {		/* %-globals, no explicit path */
	if (gloplib[0] != EOL)		/* append % global access path */
	    while ((ch = filnam[k++] = gloplib[j++]) != ':' && ch != EOL) ;
    } else if (key[1] != '/') {		/* no explicit path specified */
	if (glopath[0] != EOL)		/* append global access path */
	    while ((ch = filnam[k++] = glopath[j++]) != ':' && ch != EOL) ;
    }
    if (savj == 0 && ch == EOL)
	pathscan = FALSE;		/* one path only: inhibit search */
    if (k > 0) {
	if (k == 1 || (k == 2 && filnam[0] == '.'))
	    k = 0;
	else
	    filnam[k - 1] = '/';
    }
    savch = ch;
    savj = j;
    i = 0;
    j = 0;
    while (key[i] != EOL) {
	if ((filnam[k] = key[i]) == DELIM)
	    break;
	if (filnam[k] == '/') {
	    j = i;
	    if (k > i) {
		i = 0;
		j = 0;
		k = 0;
		continue;
	    }
	}
	i++;
	k++;
    }
    filnam[k] = NUL;			/* NUL not EOL !!! */

/* if a unix directory is specified, reposition '^' */
/* '^/usr/test' becomes '/usr/^test'                */
    if (j > 0) {
	for (k = 0; k < j; k++)
	    filnam[k] = filnam[k + 1];
	filnam[j] = '^';
    }
/* compact key to internal format: characters are shifted left */
/* delimiters become LSB of previous character                 */
/* test subscripts for being numeric or not                    */
/* numeric values are shifted into the code space              */
/* that is available because of illegal CTRL subscipts         */
    k = 0;
    if (key[i] == EOL) {		/* unsubscripted variable */
	if (action == order) {
	    g_o_val[0] = EOL;
	    ierr = NEXTER;
	    return;
	}
    } else if (key[++i] == EOL) {	/* empty (first) subscript */
	if ((action != order) && (action != query)) {
	    ierr = SBSCR;
	    return;
	}
    } else {				/* non empty subscript */
	j1 = g_numeric (&key[i]);
	while ((ch = key[i++]) != EOL) {
	    if (ch == DELIM) {
		if (k == 0) {
		    ierr = SBSCR;
		    return;
		}
		if (compactkey[--k] & 01) {
		    ierr = SBSCR;
		    return;
		}
		compactkey[k++] |= 01;
		j1 = g_numeric (&key[i]);
	    } else if (UNSIGN (ch) >= DEL) {	/* transform 8bit char to 7bit */
		compactkey[k++] = (DEL << 1);
		ch = UNSIGN (ch) - DEL;
		if (UNSIGN (ch) >= DEL) {
		    compactkey[k++] = (DEL << 1);
		    ch = UNSIGN (ch) - DEL;
		}
		compactkey[k++] = ch << 1;
	    } else if (ch < SP || ch >= DEL) {
		ierr = SBSCR;
		return;
	    }
/*no CTRLs */
	    else {
		compactkey[k++] = (j1 ? (ch << 1) & 036 : ch << 1);
	    }
	}
    }

    if (action == order) {
	if (ordercnt > 0) {
	    compactkey[k] = (k == 0 || (compactkey[k - 1] & 01) ? ALPHA : OMEGA);
	    if (k > 0)
		compactkey[k - 1] |= 01;
	    keyl = (++k);
	} else if (ordercnt == 0) {	/* no scan at all */
	    k = 0;
	    i = 0;
	    while ((ch = key[i++]) != EOL)
		if (ch == DELIM)
		    k = i;
	    stcpy (data, &key[k]);
	    g_o_val[0] = EOL;
	    return;
	} else {			/* backward scanning */
	    if (k == 0 || (compactkey[k - 1] & 01)) {
		compactkey[k] = OMEGA;
		if (k > 0)
		    compactkey[k - 1] |= 01;
		k++;
	    } else
		compactkey[k - 1] |= 01;
	    keyl = k;
	}
    } else {
	if ((keyl = k) > 0) {
	    if ((compactkey[--k] & 01) && (action != query)) {
		ierr = SBSCR;
		return;
	    }
	    compactkey[k++] |= 01;
	}
    }
    compactkey[k] = g_EOL;

/* look whether file is already open */
    tryfast = FALSE;
    ch = usage[i = j = inuse];
    while (i < NO_GLOBLS) {
	k = 0;
	while (filnam[k] == oldfil[i][k]) {
	    if (filnam[k++] == NUL) {
		filedes = olddes[i];
		if (inuse == i &&
		    action == get_sym) {
		    tryfast = TRUE;
		    if (usage[i] != (-1))
			usage[i]++;
		    if (lonelyflag)
			goto tfast2;
		    fstat (filedes, &dinf);
		    if (g_ages[i] > dinf.st_mtime)
			goto tfast2;
		    g_ages[i] = time (0L);
		    goto tfast0;
		}
		inuse = i;
		if (usage[i] != (-1))
		    usage[i]++;
		goto lock;
	    }
	}
	if (ch < 0 || (usage[i] >= 0 && usage[i] < ch))
	    ch = usage[j = i];
	if (i++ == inuse) {
	    inuse = (-1);
	    i = 0;
	}
    }
    inuse = j;
    usage[j] = 1;
/* close previous file */
    if ((filedes = olddes[j]) > 0) {
	close (filedes);
	olddes[j] = 0;
    }
    strcpy (oldfil[j], filnam);		/* save current filename */
  reopen:;
    if (rgafile[0] != EOL) {
	char    rga[256];
	short   rgax;

	strcpy (rga, filnam);
	rgax = strlen (rga);
	rga[rgax] = LF;
	rga[++rgax] = NUL;
	fputs (rga, rgaccess);
    }
    for (;;)
    {
	errno = 0;
	if ((filedes = open (filnam, 2)) != -1)
	    break;
	if (errno == EINTR)
	    continue;
	if (errno == EMFILE || errno == ENFILE) {	/* too many open files now */
	    close_all_globals ();
	    continue;
	}
	break;
    }
    if (filedes == -1) {
	usage[inuse] = 0;
	oldfil[inuse][0] = NUL;
	olddes[inuse] = 0;
	g_ages[inuse] = 0;
	if ((pathscan || errno != ENOENT) &&
	    (savch != EOL))
	    goto nextpath;		/* try next access path */
/* file not found */
	if (action != set_sym) {
	    if (errno != ENOENT) {
		ierr = PROTECT;
		return;
	    }
	    if (action == dat || action == zdata) {
		data[0] = '0';
		data[1] = EOL1;
		return;
	    }
	    data[0] = EOL1;
	    if (action == get_sym || getnflag) {
		ierr = ierr < 0 ? UNDEF - CTRLB : UNDEF;
		data[0] = EOL;
	    } else if (action == order || action == query)
		g_o_val[0] = EOL;
	    return;
	}
	if (errno != ENOENT) {
	    ierr = PROTECT;
	    return;
	}
	if (pathscan) {
	    savj = 0;
	    savch = ch = EOL;
	    pathscan = FALSE;
	    goto nextpath;
	}
	if (setop) {
	    tmp1[0] = EOL;
	    m_op (tmp1, data, setop);
	    setop = 0;
	    if (ierr > OK)
		return;
	    datal = stcpy (data, tmp1);
	}
	for (i = 0; i < BLOCKLEN; block[i++] = 0) ;	/* clear block */
	stcpy0 (&block[2], compactkey, (long) keyl);
	block[0] = keyl;		/* $length of key */
	j = i = keyl + 2;
	block[i++] = 0;
	block[i++] = 0;
	block[i++] = ROOT + 1;		/* block 1 = data */
	block[BTYP] = BOTTOM;
	block[COLLA] = 0;		/* collating sequence */
	block[OFFS] = i / 256;
	block[OFFS + 1] = i % 256;
	block[NRBLK] = 0;
	block[NRBLK + 1] = 0;
	block[NRBLK + 2] = ROOT + 1;	/* nr. of blocks */

/* create file, write_lock it and initialize root block */
	for (;;)
	{
	    errno = 0;
	    if ((filedes = creat (filnam, 0666)) != -1)
		break;
	    if (errno == EMFILE || errno == ENFILE) {
		close_all_globals ();
		continue;
	    }
	    ierr = PROTECT;
	    return;
	}
	if (lonelyflag == FALSE)
	    locking (filedes, 1, 0L);
	for (;;)
	{
	    errno = 0;
	    lseek (filedes, ROOT * BLOCKLEN, 0);
	    write (filedes, block, BLOCKLEN);
	    if (errno == 0)
		break;
	    panic ();
	}

	block[NRBLK] = 0;
	block[NRBLK + 1] = 0;
	block[NRBLK + 2] = ROOT;	/* clear */
/* copy and write length of data */
	block[j] = k = stcpy (&block[j + 1], data);
	block[i = k + j + 1] = 0;	/* clear EOL symbol */
	block[BTYP] = DATA;		/* type */
	block[OFFS] = i / 256;
	block[OFFS + 1] = i % 256;
	for (;;)
	{
	    errno = 0;
	    write (filedes, block, BLOCKLEN);
	    if (errno == 0)
		break;
	    lseek (filedes, (ROOT + 1L) * BLOCKLEN, 0);
	    panic ();
	}
	close (filedes);
	if (lonelyflag == FALSE)
	    locking (filedes, 0, 0L);	/* unlock */
/* close new file, so other users can find it */
	return;
    }
    olddes[inuse] = filedes;		/* save current filedescriptor */
/* request global for exclusive use                            */
/* odd numbered actions get read access (get_sym,data,order) 3 */
/* even ones read/write access          (set_sym,kill_sym)   1 */
  lock:;

    if (action == get_sym) {
      tfast0:;
	if (lonelyflag == FALSE)
	    locking (filedes, 3, 0L);
	if (tryfast)
	    goto tfast1;		/* try again last block */
	blknbr = oldblk = ROOT;		/* start with ROOT block */
	for (;;)
	{
	  tfast1:lseek (filedes, (long) blknbr * (long) (BLOCKLEN), 0);
	    read (filedes, block, BLOCKLEN);
	  tfast2:;
	    if ((typ = block[BTYP]) == DATA) {	/* scan data block: here we test for equality only */
		offset = UNSIGN (block[OFFS]) * 256 +
			UNSIGN (block[OFFS + 1]);
		j = UNSIGN (block[0]);
		i = 0;
		stcpy0 (key1, &block[2], j);	/* get first key */
		ch = keyl;		/* ch is a register! */
		while (i < offset) {
		    j = UNSIGN (block[i++]);	/* length of key - offset */
		    k = UNSIGN (block[i++]);	/* offset into previous entry */
#ifdef VERSNEW
		    stcpy0 (&key0[k], &block[i], j);
		    i += j;
		    j += k;
#else
		    j += k;
		    while (k < j)
			key1[k++] = block[i++];		/* get key */
#endif /* VERSNEW */
		    if (j != ch) {	/* keys have different length */
			i += UNSIGN (block[i]);
			i++;
			continue;
		    }
/* key1[j]=g_EOL; */
		    j = ch;
		    do {
			j--;
		    } while (compactkey[j] == key1[j]);		/* compare keys */
		    if (j < 0) {
			k = UNSIGN (block[i++]);
			stcpy0 (data, &block[i], k);	/* get value */
			data[k] = EOL1;	/* append EOL */
			goto quit;
		    }
		    i += UNSIGN (block[i]);
		    i++;		/* skip data */
		}
/* fast access failed. try normal path */
		if (tryfast) {
		    tryfast = FALSE;
		    goto tfast0;
		}
		ierr = ierr < 0 ? UNDEF - CTRLB : UNDEF;
		data[0] = EOL;
		goto quit;		/* variable not found */
	    } else {
		if (tryfast) {
		    tryfast = FALSE;
		    goto tfast0;
		}
		if (typ == EMPTY) {
		    if (blknbr == ROOT) {
			close (filedes);
			goto reopen;
		    }
		    ierr = DBDGD;
		    goto quit;
		}
	    }
	    scanpblk (block, &addr, &found);

	    addr += UNSIGN (block[addr]) + 2;	/* skip key */
	    if ((blknbr = UNSIGN (block[addr]) * 65536 +
		 UNSIGN (block[addr + 1]) * 256 +
		 UNSIGN (block[addr + 2])) == oldblk) {
		ierr = DBDGD;
		goto quit;
	    }
	    addr += PLEN;		/* skip data */
	    oldblk = blknbr;
	    if (ierr == INRPT)
		goto quit;
	}
    }					/* end of get_sym */
    if (lonelyflag == FALSE)
	locking (filedes, (action & 01 ? 3 : 1), 0L);

/* a KILL on an unsubscripted global deletes the entire file */
    if (action == kill_sym && compactkey[0] == g_EOL) {
	lseek (filedes, ROOT, 0);
/* note : UNIX does not tell other    */
	block[BTYP] = EMPTY;		/* jobs that a file has been unlinked */
/* as long as they keep it open.      */
/* so we mark this global as EMPTY    */
	write (filedes, block, BLOCKLEN);
	if (lonelyflag == FALSE)
	    locking (filedes, 0, 0L);	/* unlock */
	close (filedes);
	olddes[inuse] = 0;
	oldfil[inuse][0] = NUL;
	usage[inuse] = 0;
	unlink (filnam);
	return;
    }
  k_again:;				/* entry point for repeated kill operations */
/* scan tree for the proper position of key */
    blknbr = oldblk = ROOT;		/* start with ROOT block */
    trx = (-1);
    for (;;)
    {
	if (++trx >= TRLIM) {
	    ierr = STKOV;
	    goto quit;
	}
	traceblk[trx] = blknbr;
	traceadr[trx] = 0;
	lseek (filedes, (long) blknbr * (long) (BLOCKLEN), 0);
	read (filedes, block, BLOCKLEN);
	typ = block[BTYP];
	if (typ == DATA) {
	    scandblk (block, &addr, &found);
	    break;
	}
	if (typ == EMPTY) {
	    if (blknbr == ROOT) {
		close (filedes);
		goto reopen;
	    }
	    ierr = DBDGD;
	    goto quit;
	}
	scanpblk (block, &addr, &found);
	traceadr[trx] = addr;
	addr += UNSIGN (block[addr]);
	addr += 2;			/* skip key */
	if ((blknbr = UNSIGN (block[addr]) * 65536 +
	     UNSIGN (block[addr + 1]) * 256 +
	     UNSIGN (block[addr + 2])) == oldblk) {
	    ierr = DBDGD;
	    goto quit;
	}
	addr += PLEN;			/* skip data */
	oldblk = blknbr;
    }
    traceadr[trx] = addr;
    switch (action) {

    case set_sym:;

	datal = stlen (data);
	offset = UNSIGN (block[OFFS]) * 256 +
		UNSIGN (block[OFFS + 1]);

	if (found != 2) {		/* new entry */
	    if (setop) {
		tmp1[0] = EOL;
		m_op (tmp1, data, setop);
		setop = 0;
		if (ierr > OK)
		    return;
		datal = stcpy (data, tmp1);
	    }
	    needed = keyl + datal + 3;
	    if ((offset + needed) > DATALIM) {
		ret_to = 'n';		/* n=new */
		goto splitd;
	    }
	  s10:;
	    {
		long    len;		/*  insert key */
		char    key0[256];

		i = 0;
		while (i < addr) {	/* compute offset into previous entry */
		    len = UNSIGN (block[i++]);
#ifdef VERSNEW
		    k = UNSIGN (block[i++]);
		    stcpy0 (&key0[k], &block[i], len);
		    i += len;
		    key0[k + len] = g_EOL;
#else
		    len += (k = UNSIGN (block[i++]));
		    while (k < len)
			key0[k++] = block[i++];
		    key0[k] = g_EOL;
#endif /* VERSNEW */
		    i += UNSIGN (block[i]);
		    i++;		/* skip data */
		}
		k = 0;
		if (addr > 0) {
		    while (compactkey[k] == key0[k]) {
			if (key[k] == g_EOL)
			    break;
			k++;
		    }
/* do *not* fully compact numerics */
		    if ((i = UNSIGN (compactkey[k])) <= POINT) {
			while (--k >= 0 && (UNSIGN (compactkey[k]) & 01) == 0) ;
			k++;
		    }
		}
		needed -= k;
		i = (offset += needed);
		block[OFFS] = i / 256;
		block[OFFS + 1] = i % 256;
		while (i >= addr) {
		    block[i] = block[i - needed];
		    i--;
		}
#ifdef VERSNEW
		i = addr;
		block[i++] = j1 = keyl - k;
		block[i++] = k;
		stcpy0 (&block[i], &compactkey[k], j1);
		i += j1;
		block[i++] = datal;
		stcpy0 (&block[i], data, datal);
#else
		block[addr + 1] = k;
		j1 = k;
		i = addr + 2;
		while (k < keyl)
		    block[i++] = compactkey[k++];
		block[addr] = k - j1;
		addr = i++;
		k = 0;
		while (k < datal)
		    block[i++] = data[k++];
		block[addr] = k;
#endif /* VERSNEW */
	    }
	    lseek (filedes, (long) blknbr * (long) (BLOCKLEN), 0);
	    write (filedes, block, BLOCKLEN);

	    if (traceadr[trx] == 0)
		update (filedes, compactkey, keyl);
	    break;
	}
/* there was a previous entry */
	addr += UNSIGN (block[addr]);
	addr += 2;
	olddatal = UNSIGN (block[addr]);
	if (setop) {
	    stcpy0 (tmp1, &block[addr + 1], (long) olddatal);
	    tmp1[olddatal] = EOL;
	    m_op (tmp1, data, setop);
	    setop = 0;
	    if (ierr > OK)
		return;
	    datal = stcpy (data, tmp1);
	}
	if ((j1 = olddatal - datal) != 0) {
	    if (j1 > 0) {		/* surplus space */
		i = addr + datal;
		k = offset;
		offset -= j1;
		j1 += i;
		stcpy0 (&block[i], &block[j1], offset - i);
		i = offset;
		while (i < k)
		    block[i++] = 0;	/* clear area */
	    } else {			/* if (j1<0) */
/* we need more space */ if ((offset - j1) > DATALIM) {
/* block too small */
		    ret_to = 'u';	/* u=update */
		    goto splitd;
		}
	      s20:;
		i = offset;
		k = addr + olddatal;
		offset -= j1;
		j1 = offset;
		while (i > k)
		    block[j1--] = block[i--];
	    }
/* overwrite */
	    block[OFFS] = offset / 256;
	    block[OFFS + 1] = offset % 256;
	    block[addr] = datal;
	} else {			/* if nothing changes, do not write */
	    i = 0;
	    j = addr + 1;
	    while (i < datal) {
		if (block[j++] != data[i])
		    break;
		i++;
	    }
	    if (i == datal)
		goto quit;
	}
	stcpy0 (&block[++addr], data, (long) datal);

	lseek (filedes, (long) blknbr * (long) (BLOCKLEN), 0);
	write (filedes, block, BLOCKLEN);
	break;

    case dat:
	data[0] = '0';
	data[1] = EOL1;
	data[2] = EOL1;
	if (found == 2) {		/* ... advance to next entry */
	    addr += UNSIGN (block[addr]);
	    addr += 2;			/* skip key */
	    addr += UNSIGN (block[addr]);
	    addr++;			/* skip data */
	    data[0] = '1';
	} {
	    long    len;
	    char    key0[256];

/* get following entry, eventually in the next blk */
	    offset = UNSIGN (block[OFFS]) * 256 +
		    UNSIGN (block[OFFS + 1]);
	    if (addr >= offset) {
		if ((blknbr = UNSIGN (block[RLPTR]) * 65536 +
		     UNSIGN (block[RLPTR + 1]) * 256 +
		     UNSIGN (block[RLPTR + 2]))) {
		    lseek (filedes, (long) blknbr * (long) (BLOCKLEN), 0);
		    read (filedes, block, BLOCKLEN);
		    j1 = UNSIGN (block[0]);
		    i = 0;
		    j = 2;
		    while (i < j1)
			key0[i++] = block[j++];
		    key0[i] = g_EOL;
		} else
		    goto quit;
	    } else {
		i = 0;
		while (i <= addr) {	/* compute offset complete key */
		    len = UNSIGN (block[i++]);
#ifdef VERSNEW
		    k = UNSIGN (block[i++]);
		    stcpy0 (&key0[k], &block[i], len);
		    key0[k + len] = g_EOL;
		    i += len;
#else
		    len += (j = UNSIGN (block[i++]));
		    while (j < len)
			key0[j++] = block[i++];
		    key0[j] = g_EOL;
#endif /* VERSNEW */
		    i += UNSIGN (block[i]);
		    i++;		/* skip data */
		}
	    }
/* is it a descendant? */
	    if (compactkey[0] == g_EOL && key0[0] != g_EOL) {
		data[1] = data[0];
		data[0] = '1';
		break;
	    }
	    i = 0;
	    while (compactkey[i] == key0[i])
		i++;
	    if (compactkey[i] == g_EOL) {
		data[1] = data[0];
		data[0] = '1';
	    }
	}
	break;

    case order:;
	if (ordercnt < 0)
	    goto zinv;
	offset = UNSIGN (block[OFFS]) * 256 +
		UNSIGN (block[OFFS + 1]);
	if (addr >= offset) {		/* look in next block */
	    if ((blknbr = UNSIGN (block[RLPTR]) * 65536 +
		 UNSIGN (block[RLPTR + 1]) * 256 +
		 UNSIGN (block[RLPTR + 2])) == 0) {
		data[0] = EOL1;
		g_o_val[0] = EOL;
		goto quit;
	    }				/* no next block */
	    lseek (filedes, (long) blknbr * (long) (BLOCKLEN), 0);
	    read (filedes, block, BLOCKLEN);
	    scandblk (block, &addr, &found);
	} {
	    long    len;
	    int     ch0;
	    char    scratch[256];
	    char    key0[256];

	    i = 0;
	    while (i <= addr) {		/* compute offset complete key */
		len = UNSIGN (block[i++]);
		len += (j = UNSIGN (block[i++]));
		while (j < len)
		    key0[j++] = block[i++];
		key0[j] = g_EOL;
		i += UNSIGN (block[i]);
		i++;			/* skip data */
	    }
/* save data value for inspection with $V(111) */
	    i = addr;
	    i += UNSIGN (block[i]);
	    i += 2;			/* skip key */
	    j = UNSIGN (block[i++]);
	    stcpy0 (g_o_val, &block[i], j);	/* get value */
	    g_o_val[j] = EOL;		/* append EOL */

	    i = 0;
	    j = 0;
	    while ((scratch[j++] = UNSIGN (key0[i++])) != g_EOL) ;
	    if (compactkey[--keyl] == ALPHA)
		keyl++;
/* count subscripts of key */
	    i = 0;
	    j1 = 0;
	    while (i < keyl)
		if (compactkey[i++] & 01)
		    j1++;
	    i = 0;
	    j = 0;
	    k = 0;
	    while (i < keyl) {
		if (scratch[i] != compactkey[j++]) {
		    k++;
		    break;
		}
		if (scratch[i++] & 01)
		    k++;
	    }
	    if (k < j1) {
		data[0] = EOL1;
		g_o_val[0] = EOL;
		goto quit;
	    }
	    while (--i >= 0) {
		if ((scratch[i] & 01))
		    break;
	    }
	    i++;
	    j = 0;
	    while ((ch = UNSIGN (scratch[i++])) != g_EOL) {
		ch0 = (ch >= SP ? (ch >> 1) :	/* 'string' chars */
		       (ch < 20 ? (ch >> 1) + '0' :	/* 0...9          */
			(ch >> 1) + SP));	/* '.' or '-'     */
		if (ch0 == DEL) {
		    if (((ch = UNSIGN (scratch[i++])) >> 1) == DEL) {
			ch0 += DEL;
			ch = UNSIGN (scratch[i++]);
		    }
		    ch0 += (ch >> 1);
		}
		data[j++] = ch0;
		if (ch & 01)
		    break;
	    }
/* forget that data value if $d=10 */
	    if (UNSIGN (scratch[i]) != g_EOL)
		g_o_val[0] = EOL;
	    data[j] = EOL1;
	    ordercounter++;
	    if (--ordercnt > 0) {	/* repeated forward scanning */
		if (ch != g_EOL)
		    scratch[i] = g_EOL;
		stcpy0 (compactkey, scratch, i + 1);
		compactkey[i - 1] |= 01;
		compactkey[i] = OMEGA;
		keyl = i + 1;
		goto k_again;
	    }
	}
	break;

    case query:

	if (found == 2) {		/* ... advance to next entry */
	    addr += UNSIGN (block[addr]);
	    addr += 2;			/* skip key */
	    addr += UNSIGN (block[addr]);
	    addr++;			/* skip data */
	}
	offset = UNSIGN (block[OFFS]) * 256 +
		UNSIGN (block[OFFS + 1]);

	while (--ordercnt > 0) {	/* repeated forward query */
	    addr += UNSIGN (block[addr]);
	    addr += 2;			/* skip key */
	    addr += UNSIGN (block[addr]);
	    addr++;			/* skip data */
	    if (addr >= offset) {	/* look in next block */
		if ((blknbr = UNSIGN (block[RLPTR]) * 65536 +
		     UNSIGN (block[RLPTR + 1]) * 256 +
		     UNSIGN (block[RLPTR + 2])) == 0) {
		    data[0] = EOL1;
		    goto quit;		/* no next block */
		}
		lseek (filedes, (long) blknbr * (long) (BLOCKLEN), 0);
		read (filedes, block, BLOCKLEN);
		addr = 0;
		offset = UNSIGN (block[OFFS]) * 256 +
			UNSIGN (block[OFFS + 1]);
	    }
	}

	if (addr >= offset) {		/* look in next block */
	    if ((blknbr = UNSIGN (block[RLPTR]) * 65536 +
		 UNSIGN (block[RLPTR + 1]) * 256 +
		 UNSIGN (block[RLPTR + 2])) == 0) {
		if (getnflag) {
		    zref[0] = EOL;
		    ierr = ARGER;
		}
		data[0] = EOL1;
		goto quit;		/* no next block */
	    }
	    lseek (filedes, (long) blknbr * (long) (BLOCKLEN), 0);
	    read (filedes, block, BLOCKLEN);
	    addr = 0;
	} {
	    long    len;
	    char    key0[256];

	    i = 0;
	    while (i <= addr) {		/* compute offset complete key */
		len = UNSIGN (block[i++]);
		len += (j = UNSIGN (block[i++]));
		while (j < len)
		    key0[j++] = block[i++];
		key0[j] = g_EOL;
		k = i;			/* save i for getnflag processing */
		i += UNSIGN (block[i]);
		i++;			/* skip data */
	    }

	    if (getnflag) {
		int     ch0;

		i = k;
		k = UNSIGN (block[i++]);
		stcpy0 (data, &block[i], k);	/* get value */
		data[k] = EOL1;		/* append EOL */
		j = 0;
		while (zref[j] != DELIM && zref[j] != EOL)
		    j++;
		if (zref[j] == EOL)
		    zref[j] = DELIM;
		nakoffs = j;
		j++;
		i = 0;			/* make this ref $ZR */
		while ((ch = UNSIGN (key0[i++])) != g_EOL) {
		    ch0 = (ch >= SP ? (ch >> 1) :	/* 'string' chars */
			   (ch < 20 ? (ch >> 1) + '0' :		/* 0...9          */
			    (ch >> 1) + SP));	/* '.' or '-'     */
		    if (ch0 == DEL) {
			if (((ch = UNSIGN (key0[i++])) >> 1) == DEL) {
			    ch0 += DEL;
			    ch = UNSIGN (key0[i++]);
			}
			ch0 += (ch >> 1);
		    }
		    zref[j++] = ch0;
		    if (j >= 252) {
			zref[j] = EOL;
			ierr = MXSTR;
			goto quit;
		    }
		    if (ch & 01) {
			nakoffs = j;
			zref[j++] = DELIM;
		    }
		}
		zref[--j] = EOL;
		break;
	    } else {			/* save data value for inspection with $V(111) */
		int     ch0;

		i = addr;
		i += UNSIGN (block[i]);
		i += 2;			/* skip key */
		j = UNSIGN (block[i++]);
		stcpy0 (g_o_val, &block[i], j);		/* get value */
		g_o_val[j] = EOL;	/* append EOL */

		j = 0;
		i = 0;
		while ((data[j] = zref[j]) != DELIM) {
		    if (data[j] == EOL1) {
			data[j] = '(';
			break;
		    }
		    j++;
		}
		data[j++] = '(';
		k = 1;
		while ((ch = UNSIGN (key0[i++])) != g_EOL) {
		    int     typ;

		    if (k) {
			k = 0;
			if ((typ = (ch > SP)))
			    data[j++] = '"';
		    }
		    ch0 = (ch >= SP ? (ch >> 1) :	/* 'string' chars */
			   (ch < 20 ? (ch >> 1) + '0' :		/* 0...9          */
			    (ch >> 1) + SP));	/* '.' or '-'     */
		    if (ch0 == DEL) {
			if (((ch = UNSIGN (key0[i++])) >> 1) == DEL) {
			    ch0 += DEL;
			    ch = UNSIGN (key0[i++]);
			}
			ch0 += (ch >> 1);
		    }
		    data[j] = ch0;
		    if (j >= 252) {
			data[j] = EOL1;
			ierr = MXSTR;
			goto quit;
		    }
		    if (data[j++] == '"')
			data[j++] = '"';
		    if (ch & 01) {
			if (typ)
			    data[j++] = '"';
			data[j++] = ',';
			k = 1;
		    }
		}

		data[j--] = EOL1;
		data[j] = ')';
	    }
	}

	break;


    case kill_sym:;			/* search and destroy */
      killo:;				/* entry from killone section */
	offset = UNSIGN (block[OFFS]) * 256 +
		UNSIGN (block[OFFS + 1]);
	i = 0;
	key1[0] = g_EOL;
	while (i < addr) {		/* compute complete key */
	    k = UNSIGN (block[i++]);
	    k += (j = UNSIGN (block[i++]));
	    while (j < k)
		key1[j++] = block[i++];
	    key1[j] = g_EOL;
	    i += UNSIGN (block[i]);
	    i++;			/* skip data */
	}
/* look whether there's something to do at all */
	if (found != 2) {		/* is it a descendant ? */
	    char    key0[256];
	    long    trxsav;

	    if (addr >= offset) {	/* nothing to kill in that block */
		blknbr = UNSIGN (block[RLPTR]) * 65536 +
			UNSIGN (block[RLPTR + 1]) * 256 +
			UNSIGN (block[RLPTR + 2]);
		if (blknbr == 0)
		    break;		/* there is no next block */
/* maybe there's something in the next block ... */
/***************/
		trxsav = trx;
		for (;;)
		{
		    other = traceblk[--trx];
		    addr = traceadr[trx];
		    lseek (filedes, (long) other * (long) (BLOCKLEN), 0);
		    read (filedes, block, BLOCKLEN);
		    addr += UNSIGN (block[addr]);
		    addr += (2 + PLEN);	/* skip previous entry */
		    offset = UNSIGN (block[OFFS]) * 256 +
			    UNSIGN (block[OFFS + 1]);
		    traceadr[trx] = addr;
		    if (addr < offset)
			break;
		    traceadr[trx] = 0;
		    traceblk[trx] = UNSIGN (block[RLPTR]) * 65536 +
			    UNSIGN (block[RLPTR + 1]) * 256 +
			    UNSIGN (block[RLPTR + 2]);
		}
		trx = trxsav;
/***************/
		lseek (filedes, (long) blknbr * (long) (BLOCKLEN), 0);
		read (filedes, block, BLOCKLEN);
		offset = UNSIGN (block[OFFS]) * 256 +
			UNSIGN (block[OFFS + 1]);
		addr = 0;
		k = UNSIGN (block[0]);
		stcpy0 (key0, &block[2], k);
		key0[k] = g_EOL;
	    } else {			/* get following entry */
		stcpy0 (key0, key1, j);
		i = addr;
		k = UNSIGN (block[i++]);
		k += (j = UNSIGN (block[i++]));
		while (j < k)
		    key0[j++] = block[i++];
		key0[j] = g_EOL;
	    }
/* is it a descendant? */
	    i = 0;
	    while (compactkey[i] == key0[i])
		i++;
	    if (compactkey[i] != g_EOL)
		break;			/* nothing to kill */
	}
/* scan this block for all descendants */
	{
	    long    begadr,
	            endadr,
	            len;
	    char    key0[256];

	    stcpy0 (key0, compactkey, (long) keyl);

	    begadr = endadr = i = addr;
	    if (action == killone) {
		i += UNSIGN (block[i]);
		i += 2;			/* skip key */
		i += UNSIGN (block[i]);
		i++;			/* skip data */
		endadr = i;
	    } else
		while (i < offset) {
		    len = UNSIGN (block[i++]);
		    k = j = UNSIGN (block[i++]);
		    if (k >= keyl)
			i += len;
		    else {
			len += k;
			while (j < len)
			    key0[j++] = block[i++];
			key0[j] = g_EOL;
/*  k=0; ueberfluessig */
			while (compactkey[k] == key0[k]) {
			    if (compactkey[k] == g_EOL)
				break;
			    k++;
			}
			if (compactkey[k] != g_EOL)
			    break;	/* no descendant */
		    }
		    i += UNSIGN (block[i]);
		    i++;		/* skip data */
		    endadr = i;
		}
	    kill_again = (endadr == offset && action != killone);	/* may be there's more to kill */
	    if ((begadr == 0) && (endadr == offset)) {	/* block becomes empty */
		long    left,
		        right;
		char    block0[BLOCKLEN];

	      p_empty:;		/* entry if pointer block goes empty */
		left = UNSIGN (block[LLPTR]) * 65536 +
			UNSIGN (block[LLPTR + 1]) * 256 +
			UNSIGN (block[LLPTR + 2]);
		right = UNSIGN (block[RLPTR]) * 65536 +
			UNSIGN (block[RLPTR + 1]) * 256 +
			UNSIGN (block[RLPTR + 2]);
		if (left) {
		    lseek (filedes, (long) left * (long) (BLOCKLEN), 0);
		    read (filedes, block0, BLOCKLEN);
		    block0[RLPTR] = block[RLPTR];
		    block0[RLPTR + 1] = block[RLPTR + 1];
		    block0[RLPTR + 2] = block[RLPTR + 2];
		    lseek (filedes, (long) left * (long) (BLOCKLEN), 0);
		    write (filedes, block0, BLOCKLEN);
		}
		if (right) {
		    lseek (filedes, (long) right * (long) (BLOCKLEN), 0);
		    read (filedes, block0, BLOCKLEN);
		    block0[LLPTR] = block[LLPTR];
		    block0[LLPTR + 1] = block[LLPTR + 1];
		    block0[LLPTR + 2] = block[LLPTR + 2];
		    lseek (filedes, (long) right * (long) (BLOCKLEN), 0);
		    write (filedes, block0, BLOCKLEN);
		}
		b_free (filedes, blknbr);	/* modify free list */
/* delete pointer */
/**************************/
		{
		    long    trxsav;
		    long    freecnt;

		    trxsav = trx;

		    blknbr = traceblk[--trx];
		    addr = traceadr[trx];

		    lseek (filedes, (long) (blknbr) * (long) (BLOCKLEN), 0);
		    read (filedes, block, BLOCKLEN);
		    offset = UNSIGN (block[OFFS]) * 256 +
			    UNSIGN (block[OFFS + 1]);
		    freecnt = UNSIGN (block[addr]) + 2 + PLEN;
/* delete key */
		    offset -= freecnt;
		    if (offset == 0) {	/* pointer block went empty */
			if (blknbr == ROOT) {	/* global went empty */
			    lseek (filedes, 0L, 0);
/* note : UNIX does not tell other    */
			    block[BTYP] = EMPTY;	/* jobs that a file has been unlinked */
/* as long as they keep it open.      */
/* so we mark this global as EMPTY    */
			    write (filedes, block, BLOCKLEN);
			    close (filedes);
			    unlink (filnam);
			    if (lonelyflag == FALSE)
				locking (filedes, 0, 0L);	/* unlock */
			    olddes[inuse] = 0;
			    oldfil[inuse][0] = NUL;
			    usage[inuse] = 0;
			    return;
			}
			goto p_empty;	/* clear pointer block */
		    }
		    block[OFFS] = offset / 256;
		    block[OFFS + 1] = offset % 256;
		    stcpy0 (&block[addr], &block[addr + freecnt], (long) (offset - addr));
		    for (i = offset; i < offset + freecnt; block[i++] = 0) ;

		    lseek (filedes, (long) (blknbr) * (long) (BLOCKLEN), 0);
		    write (filedes, block, BLOCKLEN);
		    if (addr == 0) {	/* update of pointer */
			traceadr[trx] = 0;
			update (filedes, &block[2], (long) UNSIGN (block[0]));
		    }
		    trx = trxsav;
		}
/**************************/
		if (kill_again)
		    goto k_again;
		break;
	    }
	    i = begadr;
	    j = endadr;
	    while (j < offset)
		block[i++] = block[j++];
	    while (i < offset)
		block[i++] = 0;
/** clear rest */
	    offset += (begadr - endadr);
	    if (begadr < offset && block[begadr + 1]) {		/* new key_offset for next entry */
		i = block[begadr];
		j = block[begadr + 1];
		k = 0;
		if (begadr)
		    while (key0[k] == key1[k])
			k++;		/* new key_offset */
		if (k < j) {
		    ch = j - k;		/* space requirement */
		    block[begadr] = i + ch;	/* new key_length */
		    block[begadr + 1] = k;	/* new key_offset */
		    i = offset;
		    j = i + ch;
		    offset = j;
		    begadr++;
		    while (i > begadr)
			block[j--] = block[i--];
		    stcpy0 (&block[begadr + 1], &key0[k], ch);
		}
	    }
	    block[OFFS] = offset / 256;
	    block[OFFS + 1] = offset % 256;
	    lseek (filedes, (long) blknbr * (long) (BLOCKLEN), 0);
	    write (filedes, block, BLOCKLEN);
	    if (addr < 3) {		/* update of pointer */
		traceadr[trx] = 0;
		update (filedes, &block[2], (long) UNSIGN (block[0]));
	    }
	}

	if (kill_again)
	    goto k_again;

	break;

      zinv:;

	{
	    long    len;
	    int     ch0;
	    char    scratch[256];
	    char    key0[256];

	    if (addr <= 0) {		/* look in previous block */
		if ((blknbr = UNSIGN (block[LLPTR]) * 65536 +
		     UNSIGN (block[LLPTR + 1]) * 256 +
		     UNSIGN (block[LLPTR + 2])) == 0) {
		    data[0] = EOL1;
		    goto quit;
		}			/* no previous block */
		lseek (filedes, (long) blknbr * (long) (BLOCKLEN), 0);
		read (filedes, block, BLOCKLEN);
		addr = UNSIGN (block[OFFS]) * 256 +
			UNSIGN (block[OFFS + 1]);
	    }
	    i = 0;
	    while (i < addr) {		/* compute offset complete key */
		len = UNSIGN (block[i++]);
		len += (j = UNSIGN (block[i++]));
		while (j < len)
		    key0[j++] = block[i++];
		key0[j] = g_EOL;
		j1 = i;
		i += UNSIGN (block[i]);
		i++;			/* skip data */
	    }

/* save data value for inspection with $V(111) */
	    j = UNSIGN (block[j1++]);
	    stcpy0 (g_o_val, &block[j1], j);	/* get value */
	    g_o_val[j] = EOL;		/* append EOL */

	    i = 0;
	    j = 0;
	    while ((scratch[j++] = UNSIGN (key0[i++])) != g_EOL) ;
/* count subscripts of key */
	    i = 0;
	    j1 = 0;
	    while (i < keyl)
		if (compactkey[i++] & 01)
		    j1++;
	    i = 0;
	    j = 0;
	    k = 0;
	    while (i < keyl) {
		if (scratch[i] != compactkey[j++]) {
		    k++;
		    break;
		}
		if (scratch[i++] & 01)
		    k++;
	    }
	    if (k < j1) {
		data[0] = EOL1;
		g_o_val[0] = EOL;
		goto quit;
	    }
	    while (--i >= 0) {
		if ((scratch[i] & 01))
		    break;
	    }
	    i++;
	    j = 0;
	    while ((ch = UNSIGN (scratch[i++])) != g_EOL) {
		ch0 = (ch >= SP ? (ch >> 1) :	/* 'string' chars */
		       (ch < 20 ? (ch >> 1) + '0' :	/* 0...9          */
			(ch >> 1) + SP));	/* '.' or '-'     */
		if (ch0 == DEL) {
		    if (((ch = UNSIGN (scratch[i++])) >> 1) == DEL) {
			ch0 += DEL;
			ch = UNSIGN (scratch[i++]);
		    }
		    ch0 += (ch >> 1);
		}
		data[j++] = ch0;
		if (ch & 01)
		    break;
	    }
	    data[j] = EOL1;
	    if (j == 0)
		break;
	    ordercounter++;
	    if (ordercnt++ < (-1)) {	/* repeated backward scanning */
		if (ch != g_EOL)
		    scratch[i] = g_EOL;
		stcpy0 (compactkey, scratch, i + 1);
		compactkey[i - 1] |= 01;
		keyl = i;
		goto k_again;
	    }
	}
	break;

    case zdata:			/* nonstandard data function */

	{
	    long    counties[128];
	    char    key0[256];
	    int     icnt,
	            icnt0,
	            len;

	    i = 0;
	    while (i < 128)
		counties[i++] = 0L;	/* init count;  */
	    if (found == 2) {		/* ... advance to next entry */
		addr += UNSIGN (block[addr]);
		addr += 2;		/* skip key */
		addr += UNSIGN (block[addr]);
		addr++;			/* skip data */
		counties[0] = 1L;
	    }
	    offset = UNSIGN (block[OFFS]) * 256 +
		    UNSIGN (block[OFFS + 1]);
	    icnt = 0;
	    i = 0;
	    while ((ch = compactkey[i++]) != g_EOL)
		if (ch & 01)
		    icnt++;
	    len = i - 1;
	    i = 0;
	    while (i < addr) {		/* compute offset complete key */
		icnt0 = UNSIGN (block[i++]);
		icnt0 += (j = UNSIGN (block[i++]));
		while (j < icnt0)
		    key0[j++] = block[i++];
		key0[j] = g_EOL;
		i += UNSIGN (block[i]);
		i++;			/* skip data */
	    }

	    for (;;)			/* is it still a descendant ??? */
	    {
		if (addr >= offset) {	/* look in next block */
		    if ((blknbr = UNSIGN (block[RLPTR]) * 65536 +
			 UNSIGN (block[RLPTR + 1]) * 256 +
			 UNSIGN (block[RLPTR + 2])) == 0) {
			break;		/* no next block */
		    }
		    lseek (filedes, (long) blknbr * (long) (BLOCKLEN), 0);
		    read (filedes, block, BLOCKLEN);
		    addr = 0;
		    offset = UNSIGN (block[OFFS]) * 256 +
			    UNSIGN (block[OFFS + 1]);
		}
		i = UNSIGN (block[addr++]);
		i += (j = UNSIGN (block[addr++]));
		while (j < i)
		    key0[j++] = block[addr++];
		addr += UNSIGN (block[addr]);
		addr++;			/* skip data */
		icnt0 = 0;
		i = 0;
		while (i < j)
		    if (key0[i++] & 01)
			icnt0++;
		if (icnt0 <= icnt)
		    break;
		i = 0;
		while (i < len) {
		    if (key0[i] != compactkey[i])
			break;
		    i++;
		}
		if (i < len)
		    break;
		counties[icnt0 - icnt]++;
	    }

	    i = 128;
	    while (counties[--i] == 0L) ;
	    lintstr (data, counties[0]);
	    j = 1;
	    tmp1[0] = ',';
	    while (j <= i) {
		lintstr (&tmp1[1], counties[j++]);
		stcat (data, tmp1);
	    }
	}
	break;

    case getinc:{
	    int     setopsav;

	    setopsav = setop;
	    setop = '+';
	    data[0] = '1';
	    data[1] = EOL;
	    global  (set_sym, key, data);

	    setop = setopsav;
	    return;
	}
    case killone:
	{
	    if (found == 2)
		goto killo;		/* entry found use normal kill routine */
	    goto quit;
	}
    default:
	ierr = INVREF;			/* accidental call with wrong action code (v22-stuff) */

	//http://www.lawyersweekly.com/macoa/1102800.htm
    }					/* end of switch */
  quit:
/* clean things up */

    lseek (filedes, ROOT, 0);
    if (lonelyflag == FALSE)
	locking (filedes, 0, 0L);	/* unlock */
    return;
/**************************************************************************/
  splitd:;				/* split data block in two sections */
/* part of the data is taken to an other location.                */

/* old information in 'block' stored at 'blknbr' */
/* 'addr' there I would like to insert, if possible (which is not) */
/* 'offset' filled up to this limit */



    getnewblk (filedes, &newblk);	/* get a new block */

/* if we have to insert at the begin or end of a block  */
/* we don't split - we just start a new block           */
/* if an insert in the midst of a block, we split       */

    if (addr >= offset) {
	long    right,
	        right1,
	        right2;

	right = UNSIGN (block[RLPTR]);
	right1 = UNSIGN (block[RLPTR + 1]);
	right2 = UNSIGN (block[RLPTR + 2]);
	block[RLPTR] = newblk / 65536;
	block[RLPTR + 1] = newblk % 65536 / 256;
	block[RLPTR + 2] = newblk % 256;
	lseek (filedes, (long) blknbr * (long) (BLOCKLEN), 0);
	write (filedes, block, BLOCKLEN);
	block[RLPTR] = right;
	block[RLPTR + 1] = right1;
	block[RLPTR + 2] = right2;
	block[LLPTR] = blknbr / 65536;
	block[LLPTR + 1] = blknbr % 65536 / 256;
	block[LLPTR + 2] = blknbr % 256;
	offset = 0;
	addr = 0;
	blknbr = newblk;
	insert (filedes, compactkey, keyl, newblk);
/* up-date LL-PTR of RL-block */
	if ((other = right * 65536 + right1 * 256 + right2)) {
	    char    block0[BLOCKLEN];

	    lseek (filedes, (long) other * (long) (BLOCKLEN), 0);
	    read (filedes, block0, BLOCKLEN);
	    block0[LLPTR] = blknbr / 65536;
	    block0[LLPTR + 1] = blknbr % 65536 / 256;
	    block0[LLPTR + 2] = blknbr % 256;
	    lseek (filedes, (long) other * (long) (BLOCKLEN), 0);
	    write (filedes, block0, BLOCKLEN);
	}
	goto spltex;
    }
    if (addr == 0) {
	long    left,
	        left1,
	        left2;

	left = UNSIGN (block[LLPTR]);
	left1 = UNSIGN (block[LLPTR + 1]);
	left2 = UNSIGN (block[LLPTR + 2]);
	block[LLPTR] = newblk / 65536;
	block[LLPTR + 1] = newblk % 65536 / 256;
	block[LLPTR + 2] = newblk % 256;
	lseek (filedes, (long) blknbr * (long) (BLOCKLEN), 0);
	write (filedes, block, BLOCKLEN);
	block[LLPTR] = left;
	block[LLPTR + 1] = left1;
	block[LLPTR + 2] = left2;
	block[RLPTR] = blknbr / 65536;
	block[RLPTR + 1] = blknbr % 65536 / 256;
	block[RLPTR + 2] = blknbr % 256;
	offset = 0;
	blknbr = newblk;
	traceadr[trx] = (-1);		/* inhibit second update of pointers */
	insert (filedes, compactkey, keyl, newblk);
	if (addr < 3) {			/* update of pointer */
	    traceadr[trx] = 0;
	    update (filedes, compactkey, keyl);
	}
/* other is ***always*** zero !!!
 * if (other=left*65536+left1*256+left2) up-date RL-PTR of LL-block
 * { char block0[BLOCKLEN];
 * lseek(filedes,(long)other*(long)(BLOCKLEN),0);
 * read(filedes,block0,BLOCKLEN);
 * block0[RLPTR  ]=blknbr/65536;
 * block0[RLPTR+1]=blknbr%65536/256;
 * block0[RLPTR+2]=blknbr%256;
 * lseek(filedes,(long)other*(long)(BLOCKLEN),0);
 * write(filedes,block0,BLOCKLEN);
 * }
 */
	goto spltex;
    } {
	char    block0[BLOCKLEN];
	char    key0[256];
	int     newlimit;

	block0[BTYP] = DATA;
/* now search for a dividing line                       */
	limit = (offset + needed) / 2;
	i = (offset - needed) / 2;
	if (addr < i)
	    limit = i;

	i = 0;
	newlimit = i;
	while (i < limit) {
	    newlimit = i;
	    j = UNSIGN (block[i++]);	/* length of key - offset */
	    k = UNSIGN (block[i++]);	/* offset into previous entry */
	    j += k;
	    while (k < j)
		key0[k++] = block[i++];	/* get key */
	    key0[k] = g_EOL;
	    i += UNSIGN (block[i]);
	    i++;			/* skip data */
	}
	limit = newlimit;
	i = newlimit;

	j = i;
	i = 0;				/* copy part of old to new blk */
	if ((k = UNSIGN (block[j + 1])) != 0) {		/* expand key */
	    block0[i++] = UNSIGN (block[j++]) + k;
	    block0[i++] = 0;
	    if (addr > limit)
		addr += k;
	    j = 0;
	    while (j < k)
		block0[i++] = key0[j++];
	    j = limit + 2;
	}
	while (j < offset) {
	    block0[i++] = block[j];
	    block[j] = 0;
	    j++;
	}
	while (i <= DATALIM)
	    block0[i++] = 0;		/* clear rest of block */
	offset -= limit;
	if (k > 0)
	    offset += k;		/* new offsets */
	block[OFFS] = limit / 256;
	block[OFFS + 1] = limit % 256;
	block0[OFFS] = offset / 256;
	block0[OFFS + 1] = offset % 256;

	if (addr <= limit) {		/* save new block away */
/* update rightlink and leftlink pointers */
	    other = UNSIGN (block[RLPTR]) * 65536 +
		    UNSIGN (block[RLPTR + 1]) * 256 +
		    UNSIGN (block[RLPTR + 2]);
	    block0[RLPTR] = block[RLPTR];
	    block0[RLPTR + 1] = block[RLPTR + 1];
	    block0[RLPTR + 2] = block[RLPTR + 2];
	    block[RLPTR] = newblk / 65536;
	    block[RLPTR + 1] = newblk % 65536 / 256;
	    block[RLPTR + 2] = newblk % 256;
	    block0[LLPTR] = blknbr / 65536;
	    block0[LLPTR + 1] = blknbr % 65536 / 256;
	    block0[LLPTR + 2] = blknbr % 256;

	    lseek (filedes, (long) (newblk) * (long) (BLOCKLEN), 0);
	    write (filedes, block0, BLOCKLEN);
	    offset = limit;
/* insert new block in pointer structure */
	    insert (filedes, &block0[2], (long) UNSIGN (block0[0]), newblk);
/* up-date LL-PTR of RL-block */
	    if (other != 0) {
		lseek (filedes, (long) other * (long) (BLOCKLEN), 0);
		read (filedes, block0, BLOCKLEN);
		block0[LLPTR] = newblk / 65536;
		block0[LLPTR + 1] = newblk % 65536 / 256;
		block0[LLPTR + 2] = newblk % 256;
		lseek (filedes, (long) other * (long) (BLOCKLEN), 0);
		write (filedes, block0, BLOCKLEN);
	    }
	} else {			/* save old block away and make new block the current block */
/* update rightlink and leftlink pointers */
	    other = UNSIGN (block[RLPTR]) * 65536 +
		    UNSIGN (block[RLPTR + 1]) * 256 +
		    UNSIGN (block[RLPTR + 2]);
	    block0[RLPTR] = block[RLPTR];
	    block0[RLPTR + 1] = block[RLPTR + 1];
	    block0[RLPTR + 2] = block[RLPTR + 2];
	    block[RLPTR] = newblk / 65536;
	    block[RLPTR + 1] = newblk % 65536 / 256;
	    block[RLPTR + 2] = newblk % 256;
	    block0[LLPTR] = blknbr / 65536;
	    block0[LLPTR + 1] = blknbr % 65536 / 256;
	    block0[LLPTR + 2] = blknbr % 256;

	    lseek (filedes, (long) blknbr * (long) (BLOCKLEN), 0);
	    write (filedes, block, BLOCKLEN);
	    stcpy0 (block, block0, (long) BLOCKLEN);
	    traceadr[trx] = (addr -= limit);
	    traceblk[trx] = (blknbr = newblk);
/* insert new block in pointer structure */
	    insert (filedes, &block0[2], (long) UNSIGN (block0[0]), newblk);
/* up-date LL-PTR of RL-block */
	    if (other != 0) {
		lseek (filedes, (long) other * (long) (BLOCKLEN), 0);
		read (filedes, block0, BLOCKLEN);
		block0[LLPTR] = newblk / 65536;
		block0[LLPTR + 1] = newblk % 65536 / 256;
		block0[LLPTR + 2] = newblk % 256;
		lseek (filedes, (long) other * (long) (BLOCKLEN), 0);
		write (filedes, block0, BLOCKLEN);
	    }
	}
    }

/******************************************************************************/
  spltex:
    if (ret_to == 'n')
	goto s10;
    goto s20;
}					/* end global() */
/******************************************************************************/
static void
splitp (filedes, block, addr, offs, blknbr)	/* split pointer block in two sections */
	short   filedes;		/* global file descriptor */
	char   *block;			/* old block (which is too small) */
	long   *addr;			/* addr of entry where to insert  */
	long   *offs;			/* offset of block */
	unsigned long *blknbr;		/* number of old block */

	/* part of the data is taken to an other location.                */

	/* old information in 'block' stored at 'blknbr' */
	/* 'addr' there I would like to insert, if possible (which is not) */
	/* 'offset' filled up to this limit */

{
    char    block0[BLOCKLEN];
    long    limit;
    unsigned long newblk;
    unsigned long other;
    register i,
            j;

    getnewblk (filedes, &newblk);	/* get a new block */
    if (*blknbr == ROOT) {		/* ROOT overflow is special */
	stcpy0 (block0, block, (long) BLOCKLEN);
/* clear pointers */
	block[LLPTR] = 0;
	block[LLPTR + 1] = 0;
	block[LLPTR + 2] = 0;
	block[RLPTR] = 0;
	block[RLPTR + 1] = 0;
	block[RLPTR + 2] = 0;
/* old root block is a 'normal' block now */
/* new root points to a single block */
	i = UNSIGN (block0[0]) + 2;
	block0[i++] = newblk / 65536;
	block0[i++] = newblk % 65536 / 256;
	block0[i++] = newblk % 256;
	block0[OFFS] = i / 256;
	block0[OFFS + 1] = i % 256;
	while (i < DATALIM)
	    block0[i++] = 0;		/* clear rest */
/* update number of blocks ! */
	i = UNSIGN (block0[NRBLK]) * 65536 +
		UNSIGN (block0[NRBLK + 1]) * 256 +
		UNSIGN (block0[NRBLK + 2]) + 1;
	block0[NRBLK] = i / 65536;
	block0[NRBLK + 1] = i % 65536 / 256;
	block0[NRBLK + 2] = i % 256;

	block0[BTYP] = POINTER;
	lseek (filedes, ROOT, 0);
	write (filedes, block0, BLOCKLEN);
/* shift trace_stack */
	j = trx++;
	i = trx;
/** if (j>=TRLIM) 'global too big' */
	while (j >= 0) {
	    traceblk[i] = traceblk[j];
	    traceadr[i--] = traceadr[j--];
	}

	traceblk[0] = 0;		/*ROOT */
	traceadr[0] = 0;
	traceblk[1] = newblk;
	*blknbr = newblk;
	getnewblk (filedes, &newblk);	/* get a new block */
    }
    block0[BTYP] = block[BTYP];
/* now search for a dividing line */
    i = 0;
    limit = (*offs) / 2;
    while (i < limit) {
	i += UNSIGN (block[i]);
	i += 2;				/* skip key */
	i += PLEN;			/* skip data */
    }
/* new offsets */
    limit = i;
    i = (*offs) - limit;
    block[OFFS] = limit / 256;
    block[OFFS + 1] = limit % 256;
    block0[OFFS] = i / 256;
    block0[OFFS + 1] = i % 256;

    for (i = 0; i <= DATALIM; block0[i++] = 0) ;
    i = 0;
    j = limit;				/* copy part of old to new blk */
    while (j < (*offs)) {
	block0[i++] = block[j];
	block[j] = 0;
	j++;
    }

    if ((*addr) <= limit) {		/* save new block away */
/* update rightlink and leftlink pointers */
	other = UNSIGN (block[RLPTR]) * 65536 +
		UNSIGN (block[RLPTR + 1]) * 256 +
		UNSIGN (block[RLPTR + 2]);
	block0[RLPTR] = block[RLPTR];
	block0[RLPTR + 1] = block[RLPTR + 1];
	block0[RLPTR + 2] = block[RLPTR + 2];
	block[RLPTR] = newblk / 65536;
	block[RLPTR + 1] = newblk % 65536 / 256;
	block[RLPTR + 2] = newblk % 256;
	block0[LLPTR] = (*blknbr) / 65536;
	block0[LLPTR + 1] = (*blknbr) % 65536 / 256;
	block0[LLPTR + 2] = (*blknbr) % 256;

	lseek (filedes, (long) (newblk) * (long) (BLOCKLEN), 0);
	write (filedes, block0, BLOCKLEN);
	(*offs) = limit;
	insert (filedes, &block0[2], (long) UNSIGN (block0[0]), newblk);
/* up-date LL-PTR of RL-block */
	if (other != 0) {
	    lseek (filedes, (long) other * (long) (BLOCKLEN), 0);
	    read (filedes, block0, BLOCKLEN);
	    block0[LLPTR] = newblk / 65536;
	    block0[LLPTR + 1] = newblk % 65536 / 256;
	    block0[LLPTR + 2] = newblk % 256;
	    lseek (filedes, (long) other * (long) (BLOCKLEN), 0);
	    write (filedes, block0, BLOCKLEN);
	}
    } else {				/* save old block away and make new block the current block */
/* update rightlink and leftlink pointers */
	other = UNSIGN (block[RLPTR]) * 65536 +
		UNSIGN (block[RLPTR + 1]) * 256 +
		UNSIGN (block[RLPTR + 2]);
	block0[RLPTR] = block[RLPTR];
	block0[RLPTR + 1] = block[RLPTR + 1];
	block0[RLPTR + 2] = block[RLPTR + 2];
	block[RLPTR] = newblk / 65536;
	block[RLPTR + 1] = newblk % 65536 / 256;
	block[RLPTR + 2] = newblk % 256;
	block0[LLPTR] = (*blknbr) / 65536;
	block0[LLPTR + 1] = (*blknbr) % 65536 / 256;
	block0[LLPTR + 2] = (*blknbr) % 256;

	(*addr) -= limit;
	(*offs) -= limit;
	lseek (filedes, (long) (*blknbr) * (long) (BLOCKLEN), 0);
	write (filedes, block, BLOCKLEN);
	stcpy0 (block, block0, (long) BLOCKLEN);
	(*blknbr) = newblk;
	insert (filedes, &block0[2], (long) UNSIGN (block0[0]), newblk);
/* up-date LL-PTR of RL-block */
	if (other != 0) {
	    lseek (filedes, (long) other * (long) (BLOCKLEN), 0);
	    read (filedes, block0, BLOCKLEN);
	    block0[LLPTR] = newblk / 65536;
	    block0[LLPTR + 1] = newblk % 65536 / 256;
	    block0[LLPTR + 2] = newblk % 256;
	    lseek (filedes, (long) other * (long) (BLOCKLEN), 0);
	    write (filedes, block0, BLOCKLEN);
	}
    }
    return;
}					/* end of splitp() */
/******************************************************************************/
static void
update (filedes, ins_key, keyl)		/* update pointer */
	short   filedes;		/* file descriptor */
	char   *ins_key;		/* key to be inserted */
	long    keyl;			/* length of that key */
{
    long    offset;
    long    addr;
    unsigned long blknbr;
    char    block[BLOCKLEN];
    long    i,
            j,
            k;

    while (traceadr[trx] == 0) {	/* update of pointer blocks necessary */
	if (--trx < 0)
	    break;
	blknbr = traceblk[trx];
	addr = traceadr[trx];
	lseek (filedes, (long) blknbr * (long) (BLOCKLEN), 0);
	read (filedes, block, BLOCKLEN);
/****************************************************************/
	{
	    long    oldkeyl;

	    oldkeyl = UNSIGN (block[addr]);
	    i = addr + keyl + 1;
	    j = oldkeyl - keyl;
	    offset = UNSIGN (block[OFFS]) * 256 +
		    UNSIGN (block[OFFS + 1]);
	    if (j > 0) {		/* surplus space */
		k = offset;
		offset -= j;
		j += i;
		while (i < offset)
		    block[i++] = block[j++];
		while (i < k)
		    block[i++] = 0;	/* clear area */
	    } else if (j < 0) {		/* we need more space */
		if ((offset - j) > DATALIM)	/* block too small */
		    splitp (filedes, block, &addr, &offset, &blknbr);
		i = offset;
		offset -= j;
		j = offset;
		k = addr + oldkeyl;
		while (i > k)
		    block[j--] = block[i--];
	    }
	    block[OFFS] = offset / 256;
	    block[OFFS + 1] = offset % 256;
	    block[addr] = keyl;
/* overwrite */
	    i = 0;
	    j = (++addr);
	    block[j++] = 0;		/*!!! */
	    while (i < keyl)
		block[j++] = ins_key[i++];
/* block pointed to remains the same */
	    lseek (filedes, (long) blknbr * (long) (BLOCKLEN), 0);
	    write (filedes, block, BLOCKLEN);
	}
/****************************************************************/
	lseek (filedes, (long) blknbr * (long) (BLOCKLEN), 0);
	read (filedes, block, BLOCKLEN);
    }
    return;
}					/* end of update() */
/******************************************************************************/
static void
insert (filedes, ins_key, key_len, blknbr)	/* insert pointer */
	int     filedes;		/* file descriptor */
	char   *ins_key;		/* key to be inserted */
	long    key_len;		/* length of that key */
	unsigned long blknbr;		/* key points to this block */
{
    unsigned long blk;
    char    block[BLOCKLEN];
    long    trxsav;
    long    offset;
    long    needed;
    long    addr;
    register i,
            k;

    trxsav = trx--;
    blk = traceblk[trx];
    addr = traceadr[trx];
    lseek (filedes, (long) (blk) * (long) (BLOCKLEN), 0);
    read (filedes, block, BLOCKLEN);
    offset = UNSIGN (block[OFFS]) * 256 +
	    UNSIGN (block[OFFS + 1]);
    if (traceadr[trx + 1] != (-1)) {
	addr += UNSIGN (block[addr]);
	addr += (2 + PLEN);
    }					/* advance to next entry */
    needed = key_len + 2 + PLEN;
    if ((offset + needed) > DATALIM)
	splitp (filedes, block, &addr, &offset, &blk);
/*  insert key */
    i = (offset += needed);
    block[OFFS] = i / 256;
    block[OFFS + 1] = i % 256;
    while (i >= addr) {
	block[i] = block[i - needed];
	i--;
    }
    i = addr + 2;
    k = 0;
    while (k < key_len)
	block[i++] = ins_key[k++];
    block[addr] = k;
    block[addr + 1] = 0;		/* !!! */
    block[i++] = blknbr / 65536;
    block[i++] = blknbr % 65536 / 256;
    block[i] = blknbr % 256;

    lseek (filedes, (long) (blk) * (long) (BLOCKLEN), 0);
    write (filedes, block, BLOCKLEN);
    trx = trxsav;
    return;
}					/* end of insert() */
/******************************************************************************/
static void
b_free (filedes, blknbr)		/* mark 'blknbr' as free */
	short   filedes;		/* global file descriptor */
	unsigned long blknbr;		/* free block */
{
    char    block0[BLOCKLEN];
    unsigned long free;
    unsigned long other;
    long    i;
    long    offset;

/* mark block as empty */
    lseek (filedes, (long) (blknbr) * BLOCKLEN, 0);
    read (filedes, block0, BLOCKLEN);
    block0[BTYP] = EMPTY;
    lseek (filedes, (long) (blknbr) * BLOCKLEN, 0);
    write (filedes, block0, BLOCKLEN);

/* do we have a list of free blocks? */
    lseek (filedes, ROOT, 0);
    read (filedes, block0, BLOCKLEN);
    if ((free = UNSIGN (block0[FREE]) * 65536 +
	 UNSIGN (block0[FREE + 1]) * 256 +
	 UNSIGN (block0[FREE + 2]))) {
	for (;;)
	{
	    lseek (filedes, (long) free * (long) BLOCKLEN, 0);
	    read (filedes, block0, BLOCKLEN);
	    other = UNSIGN (block0[RLPTR]) * 65536 +
		    UNSIGN (block0[RLPTR + 1]) * 256 +
		    UNSIGN (block0[RLPTR + 2]);
	    if (other == 0)
		break;
	    free = other;
	}
	offset = UNSIGN (block0[OFFS]) * 256 +
		UNSIGN (block0[OFFS + 1]);
/* if list is full, start new page */
	if (offset > (DATALIM - PLEN)) {
	    offset -= PLEN;
	    other = UNSIGN (block0[offset]) * 65536 +
		    UNSIGN (block0[offset + 1]) * 256 +
		    UNSIGN (block0[offset + 2]);
	    block0[offset] = 0;
	    block0[offset + 1] = 0;
	    block0[offset + 2] = 0;
	    block0[OFFS] = offset / 256;
	    block0[OFFS + 1] = offset % 256;
	    block0[RLPTR] = other / 65536;
	    block0[RLPTR + 1] = other % 65536 / 256;
	    block0[RLPTR + 2] = other % 256;
	    lseek (filedes, (long) free * (long) BLOCKLEN, 0);
	    write (filedes, block0, BLOCKLEN);

	    for (i = 0; i < BLOCKLEN; block0[i++] = 0) ;	/* clear block */
	    block0[BTYP] = FBLK;
	    block0[LLPTR] = free / 65536;
	    block0[LLPTR + 1] = free % 65536 / 256;
	    block0[LLPTR + 2] = free % 256;
	    offset = 0;
	    free = other;
	}
    } else {
	getnewblk (filedes, &free);

/* set FBLK free blocks pointer */
	lseek (filedes, ROOT, 0);
	read (filedes, block0, BLOCKLEN);
	block0[FREE] = free / 65536;
	block0[FREE + 1] = free % 65536 / 256;
	block0[FREE + 2] = free % 256;
	lseek (filedes, ROOT, 0);
	write (filedes, block0, BLOCKLEN);

	for (i = 0; i < BLOCKLEN; block0[i++] = 0) ;	/* clear block */
	block0[BTYP] = FBLK;
	offset = 0;
    }
/* enter 'blknbr' */
    block0[offset++] = blknbr / 65536;
    block0[offset++] = blknbr % 65536 / 256;
    block0[offset++] = blknbr % 256;
    block0[OFFS] = offset / 256;
    block0[OFFS + 1] = offset % 256;
    lseek (filedes, (long) free * (long) BLOCKLEN, 0);
    write (filedes, block0, BLOCKLEN);
    return;
}					/* end of b_free() */
/******************************************************************************/
static void
scanpblk (block, adr, fnd)		/* scan pointer 'block' for 'compactkey' */
	char   *block;			/* 'adr' will return an adress        */
	long   *adr;			/*  2  heureka; key found at adr      */
	long   *fnd;			/*  1  not found, adr=following entry */
{
    register i = 0;
    register k;
    long    j,
            offset,
            len;
    char    key0[256];

    *adr = 0;
    offset = UNSIGN (block[OFFS]) * 256 +
	    UNSIGN (block[OFFS + 1]);
    while (i < offset)
#ifdef VERSNEW
    {
	j = i;				/* save adress of current entry */
	len = UNSIGN (block[i++]);
/*         k  =UNSIGN(block[i++]);
 * stcpy0(&key0[k],&block[i],len);
 * key0[k+len]=g_EOL;
 * ==> */ stcpy0 (key0, &block[++i], len);
	key0[len] = g_EOL;

	i += len;
#else
    {
	j = i++;			/* save adress of current entry */
/*         len=UNSIGN(block[j])+(k=UNSIGN(block[i++]));
 * ==> */ len = UNSIGN (block[j]);
	k = 0;
	i++;
	while (k < len)
	    key0[k++] = block[i++];
	key0[k] = g_EOL;
#endif /* VERSNEW */
	if (((*fnd) = g_collate (key0)) == 1)
	    return;
	*adr = j;
	if ((*fnd) == 2)
	    return;
	i += PLEN;
    }
    return;
}					/* end of scanpblk() */
/******************************************************************************/
static void
scandblk (block, adr, fnd)		/* scan 'block' for 'compactkey'      */
	char   *block;			/* 'adr' will return an adress        */
	long   *adr;			/*  2  heureka; key found at adr      */
	long   *fnd;			/*  1  not found, adr=following entry */

{
    register i = 0;
    register k;
    long    offset,
            len;
    char    key0[256];

    offset = UNSIGN (block[OFFS]) * 256 +
	    UNSIGN (block[OFFS + 1]);
    while (i < offset)
#ifdef VERSNEW
    {
	*adr = i;
	len = UNSIGN (block[i++]);
	k = UNSIGN (block[i++]);
	stcpy0 (&key0[k], &block[i], len);
	key0[k + len] = g_EOL;
	i += len;
#else
    {
	*adr = i++;
	len = UNSIGN (block[*adr]) + (k = UNSIGN (block[i++]));
	while (k < len)
	    key0[k++] = block[i++];
	key0[k] = g_EOL;
#endif /* VERSNEW */
	if (((*fnd) = g_collate (key0)) != 0)
	    return;
	i += UNSIGN (block[i]);
	i++;				/* skip data */
    }
    *adr = i;
    return;
}					/* end of scandblk() */
/******************************************************************************/
static void
getnewblk (filedes, blknbr)		/* newblk gets a new block */
	int     filedes;		/* filedescr for global access */
	unsigned long *blknbr;		/* number of new block */

{
    char    nblock[BLOCKLEN];
    unsigned long freeblks,
            no_of_blks;
    long    other;
    long    offset;

    lseek (filedes, ROOT, 0);
    read (filedes, nblock, BLOCKLEN);
    freeblks = UNSIGN (nblock[FREE]) * 65536 +
	    UNSIGN (nblock[FREE + 1]) * 256 +
	    UNSIGN (nblock[FREE + 2]);
    no_of_blks = UNSIGN (nblock[NRBLK]) * 65536 +
	    UNSIGN (nblock[NRBLK + 1]) * 256 +
	    UNSIGN (nblock[NRBLK + 2]);
    if (freeblks) {
	lseek (filedes, (long) (freeblks) * BLOCKLEN, 0);
	read (filedes, nblock, BLOCKLEN);
	offset = UNSIGN (nblock[OFFS]) * 256 +
		UNSIGN (nblock[OFFS + 1]);
	if (offset == 0) {		/* free list is empty *//* return free list blk as new block */
	    *blknbr = freeblks;
	    other = UNSIGN (nblock[RLPTR]) * 65536 +
		    UNSIGN (nblock[RLPTR + 1]) * 256 +
		    UNSIGN (nblock[RLPTR + 2]);
/* update RL-block, if any */
	    if (other) {
		lseek (filedes, (long) (other) * BLOCKLEN, 0);
		read (filedes, nblock, BLOCKLEN);
		nblock[LLPTR] = 0;
		nblock[LLPTR + 1] = 0;
		nblock[LLPTR + 2] = 0;
		lseek (filedes, (long) (other) * BLOCKLEN, 0);
		write (filedes, nblock, BLOCKLEN);
	    }
/* update ROOT block */
	    lseek (filedes, ROOT, 0);
	    read (filedes, nblock, BLOCKLEN);
	    nblock[FREE] = other / 65536;
	    nblock[FREE + 1] = other % 65536 / 256;
	    nblock[FREE + 2] = other % 256;
	    lseek (filedes, ROOT, 0);
	    write (filedes, nblock, BLOCKLEN);
	    return;
	}
	offset -= PLEN;
	*blknbr = UNSIGN (nblock[offset]) * 65536 +
		UNSIGN (nblock[offset + 1]) * 256 +
		UNSIGN (nblock[offset + 2]);
	nblock[offset] = 0;
	nblock[offset + 1] = 0;
	nblock[OFFS] = offset / 256;
	nblock[OFFS + 1] = offset % 256;
	lseek (filedes, (long) (freeblks) * BLOCKLEN, 0);
	write (filedes, nblock, BLOCKLEN);
	return;
    }
/* else ** freeblk==0 ** */
    no_of_blks++;
    nblock[NRBLK] = no_of_blks / 65536;
    nblock[NRBLK + 1] = no_of_blks % 65536 / 256;
    nblock[NRBLK + 2] = no_of_blks % 256;
    lseek (filedes, ROOT, 0);
    write (filedes, nblock, BLOCKLEN);
    *blknbr = no_of_blks;
    for (;;)
    {
	errno = 0;
	lseek (filedes, (long) (no_of_blks) * BLOCKLEN, 0);
	write (filedes, nblock, BLOCKLEN);
	if (errno == 0)
	    break;
	panic ();
    }
    return;
}					/* end of getnewblk() */
/******************************************************************************/
static short int
g_collate (t)				/* if 't' follows 'compactkey' in MUMPS collating */
	char   *t;			/* sequence a TRUE is returned                    */

{
    char   *s = compactkey;
    register chs = *s;
    register cht = *t;
    register tx = 0;
    register sx;
    short   dif;

/* the empty one is the leader! */
    if (chs == g_EOL) {
	if (cht == g_EOL)
	    return 2;
	return TRUE;
    }
    if (cht == g_EOL)
	return FALSE;

    while (cht == s[tx]) {
	if (cht == g_EOL)
	    return 2;
	cht = t[++tx];
    }					/* (s==t) */
    chs = s[tx];
    if (chs == OMEGA)
	return FALSE;
    if (chs == ALPHA)
	return cht != g_EOL;
    if (chs == g_EOL && t[tx - 1] & 01)
	return TRUE;
    if (cht == g_EOL && s[tx - 1] & 01)
	return FALSE;

/* vade retro usque ad comma */
/* the DEC ULTRIX compiler (with -O) did not understand that: */
/* if (tx>0) { tx--; while((t[tx]&01)==0) if (--tx<0) break; tx++; }   */
/* we had to write it simpler: */
    if (tx > 0) {
	tx--;
	while ((t[tx] & 01) == 0) {
	    tx--;
	    if (tx < 0)
		break;
	}
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
		return TRUE;
	} else {
	    if (cht == MINUS)
		return FALSE;
	}
	if (chs == 1 && cht == POINT)
	    return TRUE;
	if (cht == 1 && chs == POINT)
	    return FALSE;
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
	    return cht != MINUS;
	if (tx < sx)
	    return cht == MINUS;
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
	return FALSE;
    while ((dif = (UNSIGN (cht) >> 1) - (UNSIGN (chs) >> 1)) == 0) {	/* ASCII collating */
	if ((cht & 01) && ((chs & 01) == 0))
	    return FALSE;
	if ((chs & 01) && ((cht & 01) == 0))
	    return TRUE;
	chs = s[++tx];
	cht = t[tx];
    }
    if (chs == g_EOL)
	return TRUE;
    if (cht == g_EOL)
	return FALSE;
    return dif > 0;
}					/* end g_collate() */
/******************************************************************************/
static short int
g_numeric (str)
	char   *str;			/* boolean function that tests */

					/* whether str is a canonical  */
					/* numeric                     */
{
    register ptr = 0,
            ch;
    register point = 0;

    if (str[0] == '-') {
	if ((ch = str[++ptr]) == EOL || (ch == DELIM) || (ch == '0'))
	    return FALSE;
    } else if (str[0] == '0') {
	if ((ch = str[ptr + 1]) == EOL || ch == DELIM)
	    return TRUE;
	return FALSE;			/* leading zero */
    }
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
	ch = str[ptr - 2];
	if (ch == '0' ||		/* trailing zero */
	    ch == '.')
	    return FALSE;		/* trailing point */
    }
    return TRUE;
}					/* end g_numeric() */
void
close_all_globals ()
{					/* close all globals */
    register i;

    for (i = 0; i < NO_GLOBLS; i++)
	if (oldfil[i][0] != NUL) {
	    close (olddes[i]);
	    usage[i] = 0;
	    olddes[i] = 0;
	    oldfil[i][0] = NUL;
	}
    return;
}					/* end close_all_globals() */

static void
panic ()
{					/* cry for help */
    printf ("\033[s\033[25H\033[5;7mwrite needs more disk space immediately\007");
    sleep (1);
    printf ("\033[m\007\033[2K\033[u");
/* restore screen 'cause system messed up screen */
#ifdef NOWRITEM
    write_m ("\033[4~\201");
#endif /* NOWRITEM */
    return;
}					/* end panic() */

/* End of $Source: /cvsroot-fuse/gump/FreeM/src/global.c,v $ */
