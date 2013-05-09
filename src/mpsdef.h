/***
 * $Source: /cvsroot-fuse/gump/FreeM/src/mpsdef.h,v $
 * $Revision: 1.5 $ $Date: 2000/02/22 17:48:24 $
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
 * common external definitions for all mumps modules (except mumps.c)
 * 
 */

#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
/* constants definition module */
#include "mpsdef0.h"

/*      c-global variables as external definitions */

/* NOTE: this is a constant if SYSFIVE is undefined (in mumps.c) */
/* needs to be resolved: djw 3/15/99				    */
#ifdef SYSFIVE
extern  long     FreeM_timezone;              /* time zone */
#endif

extern int errno;			/* external error code for systemcalls */

extern int m_argc;			/* arguments count     */
extern char **m_argv;			/* arguments string    */
extern char **m_envp;			/* environment pointer */
extern union four_fl {
    long unsigned all;
    char    one[4];
} glvnflag;				/* [0] unique name chars          0=no limit */

					/* [1] case sensitivity flag      0=sensitive */
					/* [2] max. name+subscripts       0=no limit */
					/* [3] max. length of a subscript 0=no limit */
extern int lonelyflag;			/* single user flag */
extern int lowerflag;			/* lowercase everywhere flag */
extern int killerflag;			/* SIGTERM handling flag */
extern int huperflag;			/* SIGHUP handling flag */
extern int s_fun_flag;			/* VIEW 70: ZSORT/ZSYNTAX flag */
extern int n_fun_flag;			/* VIEW 71: ZNEXT/ZNAME flag */
extern int p_fun_flag;			/* VIEW 72: ZPREVIOUS/ZPIECE flag */
extern int d_fun_flag;			/* VIEW 73: ZDATE/ZDATA flag */
extern int zjobflag;			/* VIEW 79: old ZJOB vs. new ZJOB flag */
extern int eightbit;			/* VIEW 80: 7 vs. 8 bit flag */
extern int PF1flag;			/* VIEW 81: PF1 flag */
extern int ordercounter;		/* VIEW 82: order counter */
extern int etxtflag;			/* VIEW 83: text in $ZE flag */
extern char lvndefault[256];		/* VIEW 89: UNDEF lvn default */
extern char gvndefault[256];		/* VIEW 90: UNDEF gvn default */
extern char exfdefault[256];		/* VIEW 91: missig QUIT expr default */
extern int typemmflag;			/* VIEW 92: DEM2EUR: type mismatch error */
extern int namespace;			/* VIEW 200: namespace index */
extern int config;			/* VIEW 201: configuration index */
extern char WHR[12][4];			/* names of currencies */
extern char EUR2WHR[12][9];		/* conversion factors EUR to ... */

extern char glo_prefix[MONTH_LEN];	/* VIEW 96: global prefix */
extern char glo_ext[MONTH_LEN];		/* VIEW 97: global postfix */
extern char rou_ext[MONTH_LEN];		/* VIEW 98: routine extention */
extern long tzoffset;			/* VIEW 99: timer offset */
extern int v100;			/* VIEW 100: return value of kill */
extern char l_o_val[256];		/* VIEW 110: local $o/$q data value */
extern char g_o_val[256];		/* VIEW 111: global $o/$q data value */
extern int zsavestrategy;		/* VIEW 133: remember ZLOAD directory on ZSAVE */
extern char prompt[256];		/* prompt expression */
extern char defprompt[256];		/* default prompt string */
extern long v93;			/* VIEW 93: zkey prod. default */
extern char v93a[NO_V93][256];		/* VIEW 93: production rules   */

extern struct vtstyp *screen;		/* active screen */
extern short jour_flag;			/* journal flag 0/1/-1          */

					/* trace variables for global.c */
extern unsigned long traceblk[TRLIM];	/* trace stack - block numbers */
extern short traceadr[TRLIM];		/*             - status        */
extern short trx;			/*             - stack pointer */
extern char compactkey[256];		/* internal form of key in global.c */

extern short mcmnd;			/* mumps command letter */
extern short arg;			/* stack pointer (expr.c)     */
extern char *argstck[PARDEPTH + 1];	/* stack of pointers to       */

					/*       intermediate results */

extern long ordercnt;			/* repeater for $order/$query */
extern short setpiece;			/* =1 set$piece executing  */
extern short setop;			/* SET op flag             */
extern char rou_name[];			/* $T(+0)/$ZN routine name */
extern char *namstck;			/* routine name stack */
extern char *namptr;			/* routine name stack pointer */
extern char *framstck;			/* DO_frame stack                 */
extern char *dofrmptr;			/* DO_frame stack pointer           */
extern char zb[40];			/* $ZB last ESC_sequence */
extern char zerror[];			/* $ZERROR last error */
extern char ztrap[NESTLEVLS + 2][ZTLEN];	/* $ZTRAP to be xecuted on error */
extern short DSM2err;			/* enable normal error processing   */
extern short nesterr;			/* nesterr and callerr contain info */
extern char callerr[NESTLEVLS + 1][40];	/* about call situation at error   */
extern short zerr;			/* $ZE numeric error code */

extern char zmc[];			/* $ZMC loadable match 'controls' */
extern char zmn[];			/* $ZMN loadable match 'numeric' */
extern char zmp[];			/* $ZMP loadable match 'punctuation' */
extern char zml[];			/* $ZML loadable match 'lowercase' */
extern char zmu[];			/* $ZMU loadable match 'uppercase' */


extern char zloc[256];			/* $ZL last local reference  */
extern char zref[256];			/* $ZR last global reference */
extern short nakoffs;			/* offset to naked reference */
extern char zfunkey[44][FUNLEN];	/* $ZF function key */
extern short xpos[MAXDEV + 1];		/* $X-vector */
extern short ypos[MAXDEV + 1];		/* $Y-vector */
extern short crlf[MAXDEV + 1];		/* CR/LF flag vector               */
extern short nodelay[MAXDEV + 1];	/* nodelay flag vector             */
extern int SIflag[MAXDEV + 1];		/* SI/SO flag                      */
extern int ESCflag[MAXDEV + 1];		/* ESC flag                        */
extern short RightMargin;		/* Output Margin. Default no       */

					/*  automatic CR/LF                */
extern short InFieldLen;		/* Input Field length Def: 255 char */
extern long DSW;			/* Device Status word (Terminal)   */
extern char LineTerm[];			/* Input Line Terminator chars     */
extern char BrkKey;			/* <INTERRUPT> key Def: CTRL/C     */
extern char ug_buf[MAXDEV + 1][256];	/* ungetc-buffers                  */
extern char devopen[MAXDEV + 1];	/*  0         not open */

					/* 'r'        input    */
					/* 'w' or 'a' output   */

extern char dev[MAXDEV + 1][40];	/* names of IO devices */

extern char G0I[MAXDEV + 1][257];	/* G0 input translation table */
extern char G0O[MAXDEV + 1][257];	/* G0 output translation table */
extern char G1I[MAXDEV + 1][257];	/* G1 input translation table */
extern char G1O[MAXDEV + 1][257];	/* G1 output translation table */

extern FILE *opnfile[];
extern char act_oucpath[MAXDEV + 1][40];	/* actual path of currently used device */

extern short olddes[NO_GLOBLS];		/* filedescr of open global files */
extern char oldfil[NO_GLOBLS][40];	/* names of open global files */
extern short usage[NO_GLOBLS];		/* usage count of global files */
extern long g_ages[NO_GLOBLS];		/* last access of global files */
extern short inuse;			/* file in use */

extern short io;			/* $IO */
extern short test;			/* $TEST */
extern short pattrnflag;		/* incomplete match flag */
extern char pattrnchar;			/* incomplete match flag supplement */
extern int zsystem;			/* $ZSYSTEM return status of UNIX call */
extern short zcc;			/* $ZC (ControlC-Flag)         */
extern char *rouptr;			/* pointer to begin of routine */
extern char *roucur;			/* cursor into routine         */
extern char *rouend;			/* pointer to end of pgm       */
extern char *rouins;			/* pointer for direct mode insert */

extern short breakon;			/* BREAK enable/disable-flag   */
extern short zbreakon;			/* ZBREAK enable/disable-flag  */
extern short zbflag;			/* 'ZBREAK from terminal'-flag */
extern short zprecise;			/* $ZPRECICION of arithmetic   */
extern long random;			/* random number */
extern long ran_a;			/* random number parameter a      */
extern long ran_b;			/* random number parameter b      */
extern long ran_c;			/* random number parameter c      */
extern short usermode;			/* 0=user mode 1=programmer mode */
extern short demomode;			/* 0=no demo   1=demo mode        */
extern int d0char;			/* demomode ouput character       */
extern int d1char;			/* demomode '!'   character       */
extern short cset;			/* 0=mumps set 1='C' set flag     */
extern short hardcopy;			/* hardcopy flag */
extern short filter;			/* filter flag                    */
extern short standard;			/* 1=standard only,               */

					/* 0=non standard features enabled */
extern short ierr;
extern char errmes[MAXERR][ERRLEN];	/* error messages                  */

extern long PSIZE;			/* size of 'partition'             */
extern char *partition;
extern unsigned long alphptr[];		/* pointers into symbol table     */
extern long symlen;			/* 'lower' bound of symbol table   */
extern char *apartition;		/* alternate partition             */
extern long asymlen;			/* 'lower' bound of symbol table   */
extern unsigned long aalphptr[];	/* pointers into symbol table      */

extern short autopsize;			/* automatic increase of PSIZE     */
extern long svnlen;			/* 'lower' bound of udf_svn_tab    */
extern long UDFSVSIZ;			/* size of userdef special var tab. */
extern short autousize;			/* automatic increase of UDFSVSIZ  */
extern char *svntable;			/* udf special variable table      */
extern unsigned long svnaptr[];		/* pointers into udf_svn_tab       */
extern long NO_OF_RBUF;			/* number of routine buffers       */
extern long PSIZE0;			/* size of routine buffers         */
extern short autorsize;			/* automatic increase of PSIZE0    */
extern short aliases;			/* aliases pointer                 */
extern char ali[];			/* aliases table                   */
extern long v22ptr;			/* view 22 aliases pointer         */
extern char *v22ali;			/* view 22 aliases field           */
extern long v22size;			/* current size of aliases field   */

extern char *buff;			/* alternate buffers */
extern char code[];			/* currently interpreted code */
extern long NSIZE;			/* size of newstack                */
extern char *newstack;			/* stack for NEWed variables */
extern char *newptr;			/* pointer to NEW stack */
extern char *newlimit;			/* pointer to NEW stack end        */
extern int errfunlvl;			/* avoid wrong error message in $$FUN */
extern short nstx;			/* nest stack:       */
extern short nestc[];			/* - command (DO...) */
extern char *nestp[];			/* - cmdptr          */
extern char *nestn[];			/* - namptr          */
extern long nestr[];			/* - roucur          */
extern char *nestnew[];			/* - newptr          */
extern short repQUIT;			/* QUIT repeater     */

extern char *argptr;			/* pointer to beg of tmp storage */
extern char *s;				/* pointer to symlen_offset      */
extern char *codptr;			/* pointer within code[] */
extern char dosave[20];			/* in a FOR range save DO label  */
extern char *xdosave;

extern char varnam[];			/* variable/array/function name  */
extern char varerr[256];		/* reference in error message    */
extern char pgms[MAXNO_OF_RBUF + 1][40];	/* names of alt.pgms */
extern long ages[];			/* last call to this pgm */
extern char *ends[];			/* saved rouend-pointer */
extern char path[MAXNO_OF_RBUF][256];	/* directory where routine was loaded */

extern char glopath[PATHLEN];		/* path to access globals     */
extern char rou0path[PATHLEN];		/* routine access with DO,GOTO,JOB */
extern char rou1path[PATHLEN];		/* routine access with ZL,ZS  */

extern char gloplib[PATHLEN];		/* path to access %globals    */
extern char rou0plib[PATHLEN];		/* %routine path (DO..)       */
extern char rou1plib[PATHLEN];		/* %routine path (ZL..)       */

extern char oucpath[PATHLEN];		/* OPEN/USE/CLOSE path */
extern char rgafile[PATHLEN];		/* routine/global access protocol file */
extern char zargdefname[PATHLEN];	/* default varname for Zcommands */
extern FILE *rgaccess;			/* dto. filedes */
extern char locktab[PATHLEN];		/* file with LOCKs */
extern char zallotab[PATHLEN];		/* file with ZALLOCATE */
extern char hcpyfile[PATHLEN];		/* hardcopy file */
extern char jourfile[PATHLEN];		/* journal file */
extern FILE *jouraccess;		/* dto. filedes */

extern char curdir[];			/* current directory */
extern char zcommds[256];		/* intrinsic z-commands */
extern char zfunctions[256];		/* intrinsic z-functions */
extern char zsvn[256];			/* intrinsic z-special variables */
extern char brkaction[256];		/* action in case of BREAK       */
extern int father;			/* JOB-ID of father process      */

					  /* date types parameters */
extern char month[NO_DATETYPE][12][MONTH_LEN];	/* month names or numbers */
extern char dat1char[NO_DATETYPE][MONTH_LEN];	/* date 1st delimiter */
extern char dat2char[NO_DATETYPE][MONTH_LEN];	/* date 2nd delimmiter */
extern char dat3char[NO_DATETYPE];	/* date day justify char */
extern char dat4flag[NO_DATETYPE];	/* 0=DMY, 1=MDY, 2=YMD */
extern char dat5flag[NO_DATETYPE];	/* suppress century digits */
extern long int datGRbeg[NO_DATETYPE];	/* first day of gregorian calendar 15-OCT-1582 */
extern int datetype;			/* type for $zd special variable */

extern char tim1char[NO_TIMETYPE];	/* time 1st delimiter */
extern char tim2char[NO_TIMETYPE];	/* time 2nd delimiter */
extern char tim3char[NO_TIMETYPE];	/* time hour justify char */
extern char tim4flag[NO_TIMETYPE];	/* 0=12 Hrs 1=24 Hrs */
extern char tim5flag[NO_TIMETYPE];	/* suppress seconds */
extern int timetype;			/* type for $zt special variable */

extern jmp_buf sjbuf;			/* on timeout */
extern char *roucu0;
extern char *dofram0;

extern short forx;			/* FOR stack pointer */
extern char forvar[NESTLEVLS + 1][40],	/* FOR variable */
        forinc[NESTLEVLS + 1][40],	/* FOR increment */
        forlim[NESTLEVLS + 1][40];	/* FOR limit value */
extern short fortyp[NESTLEVLS + 1];	/* 0 = forever    1 = single,     */

					  /* 2 = unlim.iter,3 = limit iter. */
					  /* 4 =  "" +inc=1 5 =  "" + inc=1 */
extern short fori[NESTLEVLS + 1];	/* if fortyp=5 length of forlimit */

extern char *fvar;			/* current forvar */
extern char *finc;			/* current forinc */
extern char *flim;			/* current forlim */
extern short ftyp;			/* current fortyp */
extern short fi;			/* current fori   */
extern short forsw;			/* FOR switch */
extern short loadsw;			/* flag to avoid redundant loads */

					  /* after XECUTEs */
extern short promflag;			/* prompt execute flag */
extern short privflag;			/* extrinsic z-command flag */

extern short brkstk[NESTLEVLS + 1];	/* stack for BREAK information   */

extern char *cmdstack;
extern char *cmdptr;

extern short offset;
extern long timeout;
extern short timeoutms;
extern char tmp4[80];
extern short param;			/* parameter count */
extern short paramx;			/* current parameter */
extern char *ttyname ();
extern char *calloc ();
extern short level;			/* level count */
extern short nestlt[NESTLEVLS + 1];	/* stack $T / stack levelcount */
extern short pid;			/* $J = process ID */

#ifndef OLDUNIX
extern struct sigaction act;		/* signals stuff */

#endif /* OLDUNIX */

/* Macros */
/* Create a new variable of type (datatype) with space for qty instances */
#define NEW(datatype,qty)  ((datatype *)calloc(qty,sizeof(datatype)))

/* End of $Source: /cvsroot-fuse/gump/FreeM/src/mpsdef.h,v $ */
