/***
 * $Source: /cvsroot-fuse/gump/FreeM/src/mumps.c,v $
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
 * main module of mumps
 * 
 */

#include "mpsdef0.h"
#include "errmsg.h"
#include <setjmp.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
FILE   *popen ();
char   *strcpy ();
char   *strchr ();
unsigned int sleep ();
void    free ();
int     strlen ();
int     strncmp ();

#ifndef SYSFIVE
#define FreeM_timezone -3600
#else

#ifdef __CYGWIN__
#define FreeM_timezone _timezone
#else
 long FreeM_timezone;
#endif /* __CYGWIN__ */
   
#endif /* SYSFIVE */

#ifdef NEWSTACK /* stack routines */
stack_level *stack = NULL;
#endif 

/* mumps commands */
#define BREAK       'b'
#define CLOSE       'c'
#define DO          'd'
#define DO_BLOCK     2
#define ELSE        'e'
#define FOR         'f'
#define GOTO        'g'
#define HA          'h'
#define HALT        '0'
#define HANG        '1'
#define IF          'i'
#define JOB         'j'
#define KILL        'k'
#define LOCK        'l'
#define NEW         'n'
#define OPEN        'o'
#define QUIT        'q'
#define READ        'r'
#define SET         's'
#define USE         'u'
#define VIEW        'v'
#define WRITE       'w'
#define XECUTE      'x'

#define ZALLOCATE   'A'
#define ZBREAK      'B'
#define ZDEALLOCATE 'D'
#define ZGO         'G'
#define ZHALT       'H'
#define ZINSERT     'I'
#define ZJOB        'J'
#define ZLOAD       'L'
#define ZNEW        'N'
#define ZPRINT      'P'
#define ZQUIT       'Q'
#define ZREMOVE     'R'
#define ZSAVE       'S'
#define ZTRAP       'T'
#define ZWRITE      'W'
#define PRIVATE     SP

/* common definitions for all mumps modules           */
/* same as external definition in include_file mpsdef */

extern int errno;			/* external error code for systemcalls */

int     m_argc;				/* arguments count     */
char  **m_argv;				/* arguments string    */
char  **m_envp;				/* environment pointer */

					/* glvn size parameters       */
union four_fl {
    long unsigned all;
    char    one[4];
} glvnflag;				/* [0] unique name chars          0=no limit */

					/* [1] case sensitivity flag      0=sensitive */
					/* [2] max. name+subscripts       0=no limit */
					/* [3] max. length of a subscript 0=no limit */
int     lonelyflag = FALSE;		/* single user flag */
int     lowerflag = TRUE;		/* lowercase everywhere flag */
int     killerflag = TRUE;		/* SIGTERM handling flag */
int     huperflag = TRUE;		/* SIGHUP handling flag */
int     s_fun_flag = TRUE;		/* VIEW 70: ZSORT/ZSYNTAX flag */
int     n_fun_flag = TRUE;		/* VIEW 71: ZNEXT/ZNAME flag */
int     p_fun_flag = TRUE;		/* VIEW 72: ZPREVIOUS/ZPIECE flag */
int     d_fun_flag = TRUE;		/* VIEW 73: ZDATA/ZDATE flag */
int     zjobflag = TRUE;		/* VIEW 79: old ZJOB vs. new ZJOB flag */
int     eightbit = TRUE;		/* VIEW 80: 7 vs. 8 bit flag */
int     PF1flag = FALSE;		/* VIEW 81: PF1 flag */
int     ordercounter = 0;		/* VIEW 82: order counter */
int     etxtflag = FALSE;		/* VIEW 83: text in $ZE flag */
char    lvndefault[256] = "\201";	/* VIEW 89: UNDEF lvn default */
char    gvndefault[256] = "\201";	/* VIEW 90: UNDEF gvn default */
char    exfdefault[256] = "\201";	/* VIEW 91: missing QUIT expr default */
int     typemmflag = FALSE;		/* VIEW 92: EUR2DEM: type mismatch error */
int     namespace = 0;			/* VIEW 200: namespace index */
int     config = 0;			/* VIEW 201: configuration index */
char    WHR[12][4] =
{					/* names of currencies */
    "\201",
    "EUR\201",
    "ATS\201",
    "BFR\201",
    "DEM\201",
    "ESP\201",
    "FMK\201",
    "FRF\201",
    "IEP\201",
    "ITL\201",
    "NLG\201",
    "PTE\201"
};
char    EUR2WHR[12][9] =
{					/* conversion factors EUR to ... */
    "\201",				/* dont care */
    "1\201",				/* to EUR */
    "13.7603\201",			/* to ATS */
    "40.3399\201",			/* to BFR */
    "1.95583\201",			/* to DEM (DM) */
    "166.386\201",			/* to ESP */
    "5.94573\201",			/* to FMK */
    "6.55957\201",			/* to FRF (FF) */
    ".787564\201",			/* to IEP */
    "1936.27\201",			/* to ITL */
    "2.20371\201",			/* to NLG */
    "200.482\201"			/* to PTE */
};
long    v93 = 1;			/* VIEW 93: ASCII rule default */
char    v93a[NO_V93][256] =
{
/*     ASCII    */
    " :, :!,A:a,B:b,C:c,D:d,E:e,F:f,G:g,H:h,I:i,J:j,K:k,L:l,M:m,N:n,O:o,P:p,Q:q,R:r,S:s,T:t,U:u,V:v,W:w,X:x,Y:y,Z:z\201",
/* B  - BELGIAN */
    " :, :!,@:a,\\:c,{:e,}:e,\
A:a,B:b,C:c,D:d,E:e,F:f,G:g,H:h,I:i,J:j,K:k,L:l,M:m,N:n,O:o,P:p,Q:q,R:r,S:s,T:t,U:u,V:v,W:w,X:x,Y:y,Z:z\201,\200:e",
/* D  - GERMAN  */
    " \001\002 \001\002!\001\002\042\001\002#\001\002$\001\002%\001\002&\001\002\
'\001\002(\001\002)\001\002*\001\002+\001\002,\001\002-\001\002.\001\002\
/\001\002:\001\002;\001\002<\001\002=\001\002>\001\002?\001\002@\001\002\
^\001\002_\001\002`\001\002A\001a\002B\001b\002C\001c\002D\001d\002E\001e\002\
F\001f\002G\001g\002H\001h\002I\001i\002J\001j\002K\001k\002L\001l\002\
M\001m\002N\001n\002O\001o\002P\001p\002Q\001q\002R\001r\002S\001s\002\
T\001t\002U\001u\002V\001v\002W\001w\002X\001x\002Y\001y\002Z\001z\002\
{\001ae\002[\001ae\002|\001oe\002\134\001oe\002}\001ue\002]\001ue\002\
~\001ss\002\200\001e\002\201",
/* DK - DANISH  */
    " :, :!,{:ae,|:oe,}:au,~:ue,\
A:a,B:b,C:c,D:d,E:e,F:f,G:g,H:h,I:i,J:j,K:k,L:l,M:m,N:n,O:o,P:p,Q:q,R:r,S:s,T:t,U:u,V:v,W:w,X:x,Y:y,Z:z,\
[:ae,\\:oe,]:ao,^:ue,\200:e\201",
/* E  - SPANISH */
    " :, :!,|:n,}:c,ll:l,\
A:a,B:b,C:c,D:d,E:e,F:f,G:g,H:h,I:i,J:j,K:k,L:l,M:m,N:n,O:o,P:p,Q:q,R:r,S:s,T:t,U:u,V:v,W:w,X:x,Y:y,Z:z,\200:e,\
\\:n,LL:l\201",
/* F  - FRENCH  */
    " :, :!,\\:c,{:e,|:u,}:e,\
A:a,B:b,C:c,D:d,E:e,F:f,G:g,H:h,I:i,J:j,K:k,L:l,M:m,N:n,O:o,P:p,Q:q,R:r,S:s,T:t,U:u,V:v,W:w,X:x,Y:y,Z:z,\200:e,\201",
/* I  - ITALIAN */
    " :, :!,\\:c,]:e,`:u,{:a,|:o,}:e,~:i,\
A:a,B:b,C:c,D:d,E:e,F:f,G:g,H:h,I:i,J:j,K:k,L:l,M:m,N:n,O:o,P:p,Q:q,R:r,S:s,T:t,U:u,V:v,W:w,X:x,Y:y,Z:z,\200:e,\201",
/* S  - SWEDISH */
    " :, :!,`:e,{:ae,|:oe,}:ao,~:ue,\
A:a,B:b,C:c,D:d,E:e,F:f,G:g,H:h,I:i,J:j,K:k,L:l,M:m,N:n,O:o,P:p,Q:q,R:r,S:s,T:t,U:u,V:v,W:w,X:x,Y:y,Z:z,\
@:e,[:ae,\\:oe,]:ao,~:ue,\200:e,\201"
};


char    glo_prefix[MONTH_LEN] = "^\201";	/* VIEW 96: global prefix */
char    glo_ext[MONTH_LEN] = "\201";	/* VIEW 97: global postfix */
char    rou_ext[MONTH_LEN] = ".m\201";	/* VIEW 98: routine extention */
long    tzoffset = 0L;			/* VIEW 99:  timer offset     */
int     v100 = 0;			/* VIEW 100: return value of kill */
char    l_o_val[256] = "\201";		/* VIEW 110: local $o/$q data value */
char    g_o_val[256] = "\201";		/* VIEW 111: global $o/$q data value */
int     zsavestrategy = TRUE;		/* VIEW 133: remember ZLOAD directory on ZSAVE */
char    prompt[256] = "!,\">\"\201";	/* prompt expression */
char    defprompt[256] = "\012\015>\201";	/* default prompt string */

					/* vars for screen save/restore     */
struct vtstyp *screen = NULL;		/* active screen */
short   jour_flag = 0;			/* journal flag 0/1/-1              */

					/* trace vars for global module     */
unsigned long traceblk[TRLIM];		/* trace stack - block numbers      */
short   traceadr[TRLIM];		/*             - status             */
short   trx;				/*             - stack pointer      */
char    compactkey[256];		/* internal form of key in global.c */

short   mcmnd;				/* mumps command letter */
short   arg;				/* stack pointer for expr.c         */
char   *argstck[PARDEPTH + 1];		/* stack of pointers to             */

					/*       intermediate results       */

long    ordercnt = 0L;			/* repeater for $order/$query       */
short   setpiece = FALSE;		/* TRUE: set$piece executing        */
short   setop = 0;			/* SET op flag                      */
char    rou_name[256] =
{EOL};					/* $T(+0)/$ZN routine name          */
char   *namstck;			/* routine name stack               */
char   *namptr;				/* routine name stack pointer       */
char   *framstck;			/* DO_frame stack                   */
char   *dofrmptr;			/* DO_frame stack pointer           */
char    zb[40] = "\201";		/* $ZB last ESC_sequence            */
char    zerror[300] = "\201";		/* $ZE last error                   */
short   DSM2err = FALSE;		/* enable normal error processing   */
short   nesterr = 0;			/* nesterr and callerr contain info */
char    callerr[NESTLEVLS + 1][40];	/* about call situation at error    */

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

short   zerr = OK;			/* $ZE numeric error code          */
char    zloc[256] = "\201";		/* $ZL last local reference        */
char    zref[256] = "\201";		/* $ZR last global reference       */
short   nakoffs = 0;			/* offset to naked reference       */
char    zfunkey[44][FUNLEN];		/* $ZF function key */
short   xpos[MAXDEV + 1];		/* $X-vector                       */
short   ypos[MAXDEV + 1];		/* $Y-vector                       */
short   crlf[MAXDEV + 1];		/* CR/LF flag vector               */
short   nodelay[MAXDEV + 1];		/* nodelay flag vector             */

int     ESCflag[MAXDEV + 1] =
{0, 0, 0, 0, 0};			/* ESC flag                     */

short   RightMargin = 0;		/* Output Margin. Default no       */

					/* automatic CR/LF                 */
short   InFieldLen = 255;		/* Input Field length Def: 255 char */
long    DSW = BIT2 + BIT21;		/* Device Status word (Terminal)   */
char    LineTerm[32] = "\012\015\201";	/* Input Line Terminator chars     */
char    BrkKey = 3;			/* <INTERRUPT> key Def: CTRL/C     */
char    ug_buf[MAXDEV + 1][256];	/* ungetc-buffers                  */
char    devopen[MAXDEV + 1] =
{0, 0, 0, 0, 0};			/*  0         not open             */

					/* 'r'        input                */
					/* 'w' or 'a' output               */

					/* names of IO devices */
char    dev[MAXDEV + 1][40] =
{" ",					/* HOME  */
 "/usr/tmp/mout.1/a\201",		/* dev 1 */
 "/usr/tmp/mout.2/a\201",		/* dev 2 */
 "/usr/tmp/mout.3/a\201",		/* dev 3 */
 "/usr/tmp/mout.4/a\201"		/* dev 4 */
};

char    G0I[MAXDEV + 1][257];		/* G0 input translation table */
char    G0O[MAXDEV + 1][257];		/* G0 output translation table */
char    G1I[MAXDEV + 1][257];		/* G1 input translation table */
char    G1O[MAXDEV + 1][257];		/* G1 output translation table */

FILE   *opnfile[MAXDEV + 1];
char    act_oucpath[MAXDEV + 1][40] =
{"\201", "\201", "\201", "\201", "\201"};

short   olddes[NO_GLOBLS];		/* filedescr of open global files */
char    oldfil[NO_GLOBLS][40];		/* names of open global files */
long    g_ages[NO_GLOBLS];		/* last access of global files */
short   usage[NO_GLOBLS];		/* usage count of global files */
short   inuse = 0;			/* file in use */

short   io = HOME;			/* $IO */
short   test = FALSE;			/* $TEST */
short   pattrnflag = FALSE;		/* incomplete match flag */
char    pattrnchar = EOL;		/* incomplete match flag supplement */
int     zsystem = 0;			/* $ZSYSTEM return status of UNIX call */
short   zcc = FALSE;			/* $ZC (ControlC-Flag)            */

char   *rouptr;				/* pointer to begin of routine    */
char   *roucur;				/* cursor into routine            */
char   *rouend;				/* pointer to end of pgm          */
char   *rouins;				/* pointer for direct mode insert */
short   breakon = ENABLE;		/* BREAK enable/disable-flag      */
short   zbreakon = DISABLE;		/* ZBREAK enable/disable-flag     */
short   zbflag = FALSE;			/* 'ZBREAK from terminal'-flag    */
short   zprecise = 12;			/* $ZPRECISION of arithmetic      */
long    random;				/* random number seed             */
long    ran_a = 24298L;			/* random number parameter a      */
long    ran_b = 99991L;			/* random number parameter b      */
long    ran_c = 199017L;		/* random number parameter c      */

short   usermode = 1;			/* 0=user mode 1=programmer mode  */
short   demomode = FALSE;		/* 0=no demo   1=demo mode        */
int     d0char = DEL;			/* demomode ouput character       */
int     d1char = CAN;			/* demomode '!'   character       */
short   cset = FALSE;			/* 0=mumps set 1='C' set flag     */
short   hardcopy = DISABLE;		/* hardcopy flag                  */
short   filter = FALSE;			/* filter flag                    */
short   noclear = FALSE;		/* noclear flag                   */

short   standard = 0;			/* 1=standard only,               */

					/* 0=non standard features enabled */
short   ierr;				/* immediate error status          */

long    PSIZE = DEFPSIZE;		/* size of 'partition'             */
char   *partition;			/* partition                       */
long    symlen = DEFPSIZE;		/* 'lower' bound of symbol table   */
unsigned long alphptr[128] =		/* pointers into symbol table      */
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char   *apartition;			/* alternate partition             */
long    asymlen = DEFPSIZE;		/* 'lower' bound of symbol table   */
unsigned long aalphptr[128] =		/* pointers into symbol table      */
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

short   autopsize = TRUE;		/* automatic increase of PSIZE     */
long    svnlen = DEFUDFSVSIZ;		/* 'lower' bound of udf_svn_tab    */
long    UDFSVSIZ = DEFUDFSVSIZ;		/* size of userdef special var tab. */
char   *svntable;			/* udf special variable table      */
unsigned long svnaptr[128] =		/* pointers into udf_svn_tab       */
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

short   autousize = TRUE;		/* automatic increase of UDFSVSIZ  */
long    NO_OF_RBUF = DEFNO_OF_RBUF;	/* number of routine buffers       */
long    PSIZE0 = DEFPSIZE0;		/* size of routine buffers         */
short   autorsize = TRUE;		/* automatic increase of PSIZE0    */
short   aliases = 0;			/* aliases pointer                 */
char    ali[2000];			/* aliases table                   */
long    v22ptr = 0L;			/* view 22 aliases pointer         */
char   *v22ali;				/* view 22 aliases field           */
long    v22size = 0L;			/* current size of aliases field   */

/* #ifndef NEWSTACK *//* Obsoleted by new ll stack */
long    NSIZE = DEFNSIZE;		/* size of newstack                */
char   *newstack;			/* stack for NEWed variables       */
char   *newptr;				/* pointer to NEW stack            */
char   *newlimit;			/* pointer to NEW stack end        */

short   nstx = 0;			/* nest stack:       */
short   nestc[NESTLEVLS + 1];		/* - command (DO...) */
char   *nestp[NESTLEVLS + 1];		/* - cmdptr          */
char   *nestn[NESTLEVLS + 1];		/* - namptr          */
long    nestr[NESTLEVLS + 1];		/* - roucur          */
char   *nestnew[NESTLEVLS + 1];		/* - newptr          */

short   nestlt[NESTLEVLS + 1];		/* stack $T / stack levelcount */
short   brkstk[NESTLEVLS + 1];		/* stack for BREAK information */

char    ztrap[NESTLEVLS + 2][ZTLEN];	/* $ZTRAP to be xecuted on error    */
/* #endif *//* NEWSTACK */

char   *s;				/* pointer to symlen_offset        */
char   *argptr;				/* pointer to beg of tmp-storage   */

char    code[512] =
{EOL, EOL};				/* currently interpreted code      */
char   *codptr = code;			/* pointer within code[]           */

char    dosave[20];			/* in a FOR range save DO label    */
char   *xdosave;

int     errfunlvl = 0;			/* avoid wrong error message in $$FUN */
short   repQUIT = 0;			/* QUIT repeater     */

char    varnam[256];			/* variable/array/function name  */
char    varerr[256] =
{EOL};					/* reference in error message    */
char   *buff;				/* routine buffer pool           */
char    pgms[MAXNO_OF_RBUF][40];	/* names of alt.pgms             */
long    ages[MAXNO_OF_RBUF];		/* last call to this pgm         */
char   *ends[MAXNO_OF_RBUF];		/* saved rouend-pointer          */
char    path[MAXNO_OF_RBUF][256];	/* directory where routine was loaded */

char    glopath[PATHLEN] =
{EOL};					/* path to access globals        */
char    rou0path[PATHLEN] =
{EOL};					/* routine access with DO,GOTO,JOB */
char    rou1path[PATHLEN] =
{EOL};					/* routine access with ZL,ZS     */
char    gloplib[PATHLEN] =
{"../mlib\201"};			/* path to access %globals   */
char    rou0plib[PATHLEN] =
{"../mlib\201"};			/* %routine path (DO..)     */
char    rou1plib[PATHLEN] =
{"../mlib\201"};			/* %routine path (ZL..)     */
char    oucpath[PATHLEN] = "\201";	/* OPEN/USE/CLOSE path */
char    zargdefname[PATHLEN]= "%\201";  /* default varname for Z-commands */
char    rgafile[PATHLEN] = "\201";	/* routine/global access protocol file */
FILE   *rgaccess = NULL;		/* dto. filedes */
char    locktab[PATHLEN] = "/usr/tmp/locktab";	/* file with LOCKs */
char    zallotab[PATHLEN] = "/usr/tmp/locktab";		/* file with ZALLOCATE */
char    hcpyfile[PATHLEN] = "/usr/tmp/hardcopy";	/* hardcopy file */
char    jourfile[PATHLEN] = "/usr/tmp/ioprotocol";	/* journal file */
FILE   *jouraccess;			/* dto. filedes */
char    curdir[256] = ".";		/* current directory */

char    startuprou[PATHLEN] = "\201";   /* start up routine from cmdline*/

char    zcommds[256] =
" za zb zd zg zh zi zj zl zn zp zq zr zs zt zw zallocate zbreak zdeallocate \
zgo zhalt zinsert zjob zload znew zprint zquit zremove zsave ztrap zwrite \201";	/* intrinsic z-commands */
char    zfunctions[256] =		/* intrinsic z-functions */
" zb zc zd ze zh zht zk zl zm zn zo zp zr zs zt zboolean zcall zcr zcrc zdata zdate zedit zhorolog \
zkey zlength zlsd zname znext zorder zpiece zprevious zreplace zsyntax zsort ztime zzip \201";
char    zsvn[256] =			/* intrinsic z-special variables */
" za zb zc zd ze zf zh zi zj zl zmc zmn zmp zma zml zmu zme zn zo zp zr zs zt zv \
zcontrolc zdate zerror zname zhorolog zinrpt zjob zlocal zorder zprecision zsystem ztime ztr ztrap zreference zversion \201";
char    brkaction[256] = "\201";	/* action in case of BREAK     */
int     father = 0;			/* JOB-ID of father process         */

					/* date types parameters */
char    month[NO_DATETYPE][12][MONTH_LEN] =
{
    {"01\201", "02\201", "03\201", "04\201", "05\201", "06\201", "07\201", "08\201", "09\201", "10\201", "11\201", "12\201"},
    {"01\201", "02\201", "03\201", "04\201", "05\201", "06\201", "07\201", "08\201", "09\201", "10\201", "11\201", "12\201"},
    {"JAN\201", "FEB\201", "MAR\201", "APR\201", "MAY\201", "JUN\201", "JUL\201", "AUG\201", "SEP\201", "OCT\201", "NOV\201", "DEC\201"},
    {"01\201", "02\201", "03\201", "04\201", "05\201", "06\201", "07\201", "08\201", "09\201", "10\201", "11\201", "12\201"},
    {"1\201", "2\201", "3\201", "4\201", "5\201", "6\201", "7\201", "8\201", "9\201", "10\201", "11\201", "12\201"},
    {"1\201", "2\201", "3\201", "4\201", "5\201", "6\201", "7\201", "8\201", "9\201", "10\201", "11\201", "12\201"},
    {"1\201", "2\201", "3\201", "4\201", "5\201", "6\201", "7\201", "8\201", "9\201", "10\201", "11\201", "12\201"},
    {"01\201", "02\201", "03\201", "04\201", "05\201", "06\201", "07\201", "08\201", "09\201", "10\201", "11\201", "12\201"}
};
char    dat1char[NO_DATETYPE][MONTH_LEN] =	/* date 1st delimiter */
{"/\201", "/\201", " \201", "/\201", ".\201", ".\201", ".\201", ".\201"};
char    dat2char[NO_DATETYPE][MONTH_LEN] =	/* date 2nd delimmiter */
{"/\201", "/\201", " \201", "/\201", ".\201", ".\201", ".\201", ".\201"};
char    dat3char[NO_DATETYPE] =
{'0', '0', '0', '0', '\201', '\201', '\201', '0'};	/* date day justify char */
char    dat4flag[NO_DATETYPE] =
{2, 1, 0, 0, 0, 0, 0, 0};		/* 0=DMY, 1=MDY, 2=YMD */
char    dat5flag[NO_DATETYPE] =
{0, 1, 1, 1, 1, 1, 0, 1};		/* suppress century digits */
long int datGRbeg[NO_DATETYPE] =
{578101L, 578101L, 578101L, 578101L, 578101L, 578101L, 578101L, 578101L};

							  /* first day of gregorian calendar 15-OCT-1582 ($H+672411) */
int     datetype = 0;			/* type for $zd special variable */

char    tim1char[NO_TIMETYPE] =
{':', ':'};				/* time 1st delimiter */
char    tim2char[NO_TIMETYPE] =
{':', ':'};				/* time 2nd delimiter */
char    tim3char[NO_TIMETYPE] =
{SP, SP};				/* time hour justify char */
char    tim4flag[NO_TIMETYPE] =
{0, 1};					/* 0=24 Hrs 1=12 Hrs */
char    tim5flag[NO_TIMETYPE] =
{0, 0};					/* suppress seconds */
int     timetype = 0;			/* type for $zt special variable */

jmp_buf sjbuf;
char   *roucu0;
char   *dofram0;

short   forx = 0;			/* FOR stack pointer */
char    forvar[NESTLEVLS + 1][40],	/* FOR variable */
        forinc[NESTLEVLS + 1][40],	/* FOR increment */
        forlim[NESTLEVLS + 1][40];	/* FOR limit value */
short   fortyp[NESTLEVLS + 1];		/* 0 = forever    1 = single,     */

					/* 2 = unlim.iter,3 = limit iter. */
					/* 4 =  "" +inc=1 5 =  "" + inc=1 */
short   fori[NESTLEVLS + 1];		/* if fortyp=5 length of forlimit */

char   *fvar;				/* current forvar */
char   *finc;				/* current forinc */
char   *flim;				/* current forlim */
short   ftyp;				/* current fortyp */
short   fi;				/* current fori   */
short   forsw = FALSE;			/* FOR switch */
short   loadsw = TRUE;			/* flag to avoid redundant loads */

					/* after XECUTEs */
short   promflag = TRUE;		/* prompt execute flag */
short   privflag = FALSE;		/* extrinsic z-command flag */


char   *cmdstack;
char   *cmdptr;

short   offset;
long    timeout;
short   timeoutms;
char    tmp4[80] = "\201";
short   param = 0;			/* parameter count */
short   paramx = 0;			/* current parameter */
char   *ttyname ();
char   *calloc ();
short   level = 0;			/* level count */
short   pid;				/* $J = process ID */

#ifdef	USE_SIGACTION
struct sigaction act = {0};		/* signals stuff */
#endif/*USE_SIGACTION*/

int
main (argc, argv, envp)
	int     argc;			/* arguments count     */
	char  **argv;			/* arguments string    */
	char  **envp;			/* environment pointer */

{

    register i;
    register j;
    register ch;

#ifdef DEBUG_NEWPTR
    int loop;
#endif
#ifdef NEWSTACK
    stack_level *tmp_stack;
#endif

    m_argc = argc;			/* save arguments count     */
    m_argv = argv;			/* save arguments string    */
    m_envp = envp;			/* save environment pointer */

    pid = getpid ();			/* get $J = process ID */
    setbuf (stdin, NULL);		/* no input buffering */
    glvnflag.all = 0L;

    printf("mumps start\n");

    {
	struct tm lt, gt;
	unsigned long gmt, lmt;
	unsigned long clock;

#ifdef __CYGWIN__
       tzset();                        /* may be required in order   */
       				       /* to guarantee _timezone set */
#endif /* __CYGWIN__ */

	clock = time (0L);
	lt = *localtime (&clock);
	gt = *gmtime (&clock);

	/* This is awkward but I think it is portable: steve_morris */
	gmt = gt.tm_year * 365;
	gmt = (gmt + gt.tm_yday) * 24;
	gmt = (gmt + gt.tm_hour) * 60;
	gmt = (gmt + gt.tm_min);
	
	lmt = lt.tm_year * 365;
	lmt = (lmt + lt.tm_yday) * 24;
	lmt = (lmt + lt.tm_hour) * 60;
	lmt = (lmt + lt.tm_min);
	
	FreeM_timezone = (gmt - lmt) * 60;
    }
    
    tzoffset = -FreeM_timezone;

    umask (0);				/* protection bits mask to full rights */

    strcpy (zb, argv[0]);		/* name with which mumps has been called */
    zb[strlen (zb)] = EOL;

    {					/* Import FREEM_LIB if present */
    	char   *lib, *p, libpath[STRLEN + 2];
	int     cnt;

	lib = NULL;
	cnt = 0;
	while ((p = envp[cnt]) && (p[0] != NUL)) {
	    if (strncmp("FREEM_LIB", p, 9) == 0) {
	    	lib = strchr(p, '=');
		if (lib) {
			++lib;
			break;
		}
	    }
	    ++cnt;
	}
	if (lib && (lib[0] == '/')) {
	    fprintf(stderr, "not handling root based library yet\n");
	}
	if (lib && (lib[0] != '/')) {
	    strcpy(libpath, lib);
	    strcat(libpath, "/.");
	    if (access(lib, X_OK) == 0) {
	    	strcpy(libpath, lib);
		strcat(libpath, "\201");
		stcpy(gloplib, libpath);
		stcpy(rou0plib, libpath);
		stcpy(rou1plib, libpath);
	    } else
		perror(lib);
	}
    }

    for (j = 0; j <= MAXDEV; j++) {	/* init. translation tables */
	for (i = 0; i < 256; i++) {
	    G0I[j][i] = (char) i;
	    G0O[j][i] = (char) i;
	    G1I[j][i] = (char) i;
	    G1O[j][i] = (char) i;
	}
	G0I[j][UNSIGN (EOL)] = NUL;
	G0O[j][UNSIGN (EOL)] = NUL;
	G1I[j][UNSIGN (EOL)] = NUL;
	G1O[j][UNSIGN (EOL)] = NUL;
	G0I[j][UNSIGN (DELIM)] = NUL;
	G0O[j][UNSIGN (DELIM)] = NUL;
	G1I[j][UNSIGN (DELIM)] = NUL;
	G1O[j][UNSIGN (DELIM)] = NUL;
	G0I[j][256] = EOL;
	G0O[j][256] = EOL;
	G1I[j][256] = EOL;
	G1O[j][256] = EOL;
    }
#ifdef SCO
#ifndef HACK_NOXLATE
    G0I[HOME][245] = 64;
    G0O[HOME][64] = 245;		/* Paragraph */
    G0I[HOME][142] = 91;
    G0O[HOME][91] = 142;		/* A umlaut */
    G0I[HOME][153] = 92;
    G0O[HOME][92] = 153;		/* O umlaut */
    G0I[HOME][154] = 93;
    G0O[HOME][93] = 154;		/* U umlaut */
    G0I[HOME][132] = 123;
    G0O[HOME][123] = 132;		/* a umlaut */
    G0I[HOME][148] = 124;
    G0O[HOME][124] = 148;		/* o umlaut */
    G0I[HOME][129] = 125;
    G0O[HOME][125] = 129;		/* u umlaut */
    G0I[HOME][225] = 126;
    G0O[HOME][126] = 225;		/* sharp s  */
#endif/*HACK_NOXLATE*/

/* DEC Special graphics                             */
    G1I[HOME][254] = 96;
    G1O[HOME][96] = 254;		/* diamond  */
    G1I[HOME][176] = 97;
    G1O[HOME][97] = 176;		/* checker board */
    G1I[HOME][241] = 99;
    G1O[HOME][99] = 241;		/* FF */
    G1I[HOME][242] = 100;
    G1O[HOME][100] = 242;		/* CR */
    G1I[HOME][243] = 101;
    G1O[HOME][101] = 243;		/* LF */
    G1I[HOME][248] = 102;
    G1O[HOME][102] = 248;		/* degree sign */
    G1I[HOME][241] = 103;
    G1O[HOME][103] = 241;		/* plus minus */
    G1I[HOME][244] = 104;
    G1O[HOME][104] = 244;		/* NL */
    G1I[HOME][251] = 105;
    G1O[HOME][105] = 251;		/* VT */
    G1I[HOME][217] = 106;
    G1O[HOME][106] = 217;		/* lower right corner */
    G1I[HOME][191] = 107;
    G1O[HOME][107] = 191;		/* upper right corner */
    G1I[HOME][218] = 108;
    G1O[HOME][108] = 218;		/* upper left corner */
    G1I[HOME][192] = 109;
    G1O[HOME][109] = 192;		/* lower left corner */
    G1I[HOME][197] = 110;
    G1O[HOME][110] = 197;		/* cross */
    G1I[HOME][200] = 111;
    G1O[HOME][111] = 200;		/* linescan 5 */
    G1I[HOME][201] = 112;
    G1O[HOME][112] = 201;		/* linescan 4 */
    G1I[HOME][196] = 113;
    G1O[HOME][113] = 196;		/* linescan 3 */
    G1I[HOME][202] = 114;
    G1O[HOME][114] = 202;		/* linescan 2 */
    G1I[HOME][203] = 115;
    G1O[HOME][115] = 203;		/* linescan 1 */
    G1I[HOME][195] = 116;
    G1O[HOME][116] = 195;		/* left  junction */
    G1I[HOME][180] = 117;
    G1O[HOME][117] = 180;		/* right junction */
    G1I[HOME][193] = 118;
    G1O[HOME][118] = 193;		/* lower junction */
    G1I[HOME][194] = 119;
    G1O[HOME][119] = 194;		/* upper junction */
    G1I[HOME][179] = 120;
    G1O[HOME][120] = 179;		/* vertival bar */
    G1I[HOME][243] = 121;
    G1O[HOME][121] = 243;		/* lower equals */
    G1I[HOME][242] = 122;
    G1O[HOME][122] = 242;		/* greater equals */
    G1I[HOME][227] = 123;
    G1O[HOME][123] = 227;		/* pi */
    G1I[HOME][246] = 124;
    G1O[HOME][124] = 246;		/* not equals */
    G1I[HOME][128] = 125;
    G1O[HOME][125] = 128;		/* euro sign */
    G1I[HOME][250] = 126;
    G1O[HOME][126] = 250;		/* centered dot */
#endif /* SCO */

    codptr = code;
    code[0] = EOL;			/* init code_pointer */
    partition = calloc ((unsigned) (PSIZE + 1), 1);
    if (partition == NULL)
	exit (2);			/* could not allocate stuff...     */
    symlen = PSIZE;
    s = &partition[PSIZE] - 256;	/* pointer to symlen_offset        */
    argptr = partition;			/* pointer to beg of tmp-storage   */

    svntable = calloc ((unsigned) (UDFSVSIZ + 1), 1);
    if (svntable == NULL)
	exit (2);			/* could not allocate stuff...     */
    svnlen = UDFSVSIZ;			/* begin of udf_svn_table         */
    buff = calloc ((unsigned) NO_OF_RBUF * (unsigned) PSIZE0, 1);	/* routine buffer pool          */
    if (buff == NULL)
	exit (2);			/* could not allocate stuff...     */

#ifndef NEWSTACK
    newstack = calloc ((unsigned) NSIZE, 1); 
    if (newstack == NULL)
	exit (2);			/* could not allocate stuff...     */
#ifdef DEBUG_NEWPTR
    printf("Allocating newptr stack...\r\n");
#endif
    newptr = newstack;
    newlimit = newstack + NSIZE - 1024;
#endif /* NEWSTACK */
    namstck = calloc ((unsigned) NESTLEVLS * 13, 1);
    if (namstck == NULL)
	exit (2);			/* could not allocate stuff...     */
    *namstck = EOL;
    *(namstck + 1) = EOL;
    namptr = namstck;			/* routine name stack pointer       */
    framstck = calloc ((unsigned) NESTLEVLS * 256, 1);
    if (framstck == NULL)
	exit (2);			/* could not allocate stuff...     */
    *framstck = EOL;
    *(framstck + 1) = EOL;
    dofrmptr = framstck;		/* DO_frame stack pointer           */
    cmdstack = calloc ((unsigned) NESTLEVLS * 256, 1);
    if (cmdstack == NULL)
	exit (2);			/* could not allocate stuff...     */
    cmdptr = cmdstack;			/* command stack */

    j = 0;
    while (--argc > 0) {
	j++;
        if (argv[j][0] == '^') {
            strcpy(startuprou,argv[j]);
	    startuprou[strlen(startuprou)]='\201';
	}
	/* ignore options with --name */
	if (argv[j][0] == '-' && (argv[j][1] != '-' )) {
	    i = 0;
	    while ((ch = argv[j][++i]) != 0) {
		if (ch == 'h')
		    hardcopy = ENABLE;
		if (ch == 'f')
		    filter = TRUE;
		if (ch == 'C')
		    noclear = TRUE;
		if (ch == 'd') {
		    demomode = TRUE;
		    if (argv[j][++i] == NUL)
			break;
		    d0char = argv[j][i];
		    if (argv[j][++i] == NUL)
			break;
		    d1char = argv[j][i];
		}
		if (ch == 'a')
		    usermode = 0;
		if (ch == 's')
		    standard = TRUE;
		if (ch == 'i') {	/* import environment */
		    short   cnt = 0,
		            k,
		            l;
		    char    tmp[256];

		    while (envp[cnt] && envp[cnt][0] != NUL) {
			if (strlen(envp[cnt]) > STRLEN) {
/*ldl hack*/			/* Skip any env var that is too long */
			    ++cnt;
			    continue;
			}
			strcpy (tmp, envp[cnt]);
			k = 0;
			while (tmp[k] != '=') {
			    varnam[k] = tmp[k];
			    k++;
			}
			varnam[k] = EOL;
			l = ++k;
			while (tmp[l] != NUL)
			    l++;
			tmp[l] = EOL;
			symtab (set_sym, varnam, &tmp[k]);
			cnt++;
		    }
		}
	    }
	} else
	    param++;
    }
    if (standard)
	cset = FALSE;			/* -s makes -c dummy */

/* initialize screen */

    stcpy (buff, "\201");
    writeHOME (buff);
    for (i = 0; i <= MAXDEV; ug_buf[i++][0] = EOL) ;	/* init read-buffers */

    crlf[HOME] = filter;

    rouend = rouins = rouptr = buff;
    roucur = buff + (NO_OF_RBUF * PSIZE0 + 1);
    *rouptr = EOL;
    *(rouptr + 1) = EOL;
    *(rouptr + 2) = EOL;

    if (hardcopy)
	zbreakon = ENABLE;		/* enable CTRL/B */

    set_io (MUMPS);			/* set i/o parameters */
/* init random number */
    if ((random = time (0L) * getpid ()) < 0)
	random = (-random);

    if (ttyname (HOME)) {		/* for $IO of HOME */
	strcpy (dev[HOME], ttyname (HOME));
	dev[HOME][strlen (dev[HOME])] = EOL;
    } else
	dev[HOME][0] = EOL;		/* ...we are in a pipe */
#ifdef NEWSTACK
#ifdef DEBUG_NEWSTACK
    printf("Setting Ztrap\r\n"); 
#endif
    if(!stack) {
#ifdef DEBUG_NEWSTACK
    printf("Allocating new Stack\r\n"); 
#endif
          tmp_stack = (stack_level *)calloc(1,sizeof(stack_level)); 
          tmp_stack->previous = stack; stack = tmp_stack;
    }
    if (filter) stack->ztrap[0] = EOL;
    else if (startuprou[0] == '^') stcpy (stack->ztrap, startuprou);
    else stcpy (stack->ztrap, "^%CLPM\201");/* $ZT to be xecuted on startup */
#else
    if (filter) 
	ztrap[0][0] = EOL;		/* no default ztrap for filters */
    else
/* start-up routine */
	if (startuprou[0] == '^') stcpy (ztrap[0], startuprou);
	else stcpy (ztrap[0], "^%CLPM\201");/* $ZT to be xecuted on startup */
    stcpy (ztrap[NESTLEVLS + 1], ztrap[0]);	/* DSM V.2 error trapping */
#endif
/* init function keys */
    for (i = 0; i < 44; zfunkey[i++][0] = EOL) ;

/* signals stuff */

    SIGNAL_ACTION(SIGHUP, onhup, NULL);		/* catch hangup      */
    SIGNAL_ACTION(SIGINT, onintr, NULL);	/* set_up INTERRUPT  */
    if (hardcopy) {
	SIGNAL_ACTION(SIGQUIT, hardcpf, NULL);	/* set_up hardcopy   */
    } else {
	SIGNAL_ACTION(SIGQUIT, onquit, NULL);	/* set_up ZBREAK     */
    }
    SIGNAL_ACTION(SIGTERM, onkill, NULL);	/* catch kill signal */

    /* SIGNAL_ACTION(SIGBUS, onbus, NULL); */		/* catch bus error   */
#ifndef __CYGWIN__
    SIGNAL_ACTION(SIGIOT, onbus, NULL);		/* catch IOT error   */
#endif/*__CYGWIN__*/
#ifndef LINUX
    SIGNAL_ACTION(SIGEMT, onbus, NULL);		/* catch EMT error   */
#endif/*LINUX*/
    /* SIGNAL_ACTION(SIGSEGV, onbus, NULL); */	/* catch bus error   */
    SIGNAL_ACTION(SIGUSR1, oncld, NULL);	/* catch son dies signal */
    SIGNAL_ACTION(SIGFPE, onfpe, NULL);		/* catch floating pt except */

/* initialize screen */
    if (filter == FALSE && noclear == FALSE)
	write_m ("\017\033[H\033[J\033[4~\201");
/* run mumps */
    xecline (1);
    exit (0);				/* we should never reach that statement */

}					/*end of main() */

void
unnew ()
{					/* un-NEW variables */
    char   *xptr;
    int     i;
    long    j;
    char    tmp[256];
#ifdef NEWSTACK
    stack_cmd *tmp_cmd, *free_cmd;
#endif
#ifdef DEBUG_NEWPTR
    int loop;
    printf("Un-Newing: ");
    printf("[nstx] nstx is %d\r\n",nstx);
    printf("[nestnew] nestnew[nstx] is %d\r\n",nestnew[nstx]);
#endif
#ifdef NEWSTACK
    if(!stack) { return; }
    tmp_cmd = stack->new;
    while(tmp_cmd) {
      /* UN-Implemented special stuff */
      if(tmp_cmd->command == STACK_NUKE) { /* Un-Implemented */ }
      else if(tmp_cmd->command == STACK_POP) { 
        symtab(pop_sym, tmp_cmd->key, NULL);
      } else if(tmp_cmd->command == STACK_KILL) {
        symtab(kill_sym, tmp_cmd->key, NULL);
      } else if(tmp_cmd->command == STACK_POPALL) {
        symtab(megapop, NULL, NULL);
      } else if(tmp_cmd->command == STACK_SET) {
        symtab(set_sym, tmp_cmd->key, tmp_cmd->data);
      } else { printf("Unknown command in mumps.c::Unnew\r\n"); }
      free_cmd = tmp_cmd; tmp_cmd = tmp_cmd->next;
      stack->new = tmp_cmd;
    }
    return;
#else  /* NEWSTACK */
    
    xptr = nestnew[nstx];		/* get position of newpointer */
    while (xptr < newptr) {
	i = *--newptr;
	if (i != kill_all) {
	    j = UNSIGN (*--newptr);
	    newptr -= (j + 1);
	    stcpy0 (varnam, newptr, j + 1);

	    if (i == set_sym) {
		j = UNSIGN (*--newptr);
		newptr -= (j + 1);
		stcpy (tmp, newptr);
	    } else
		tmp[0] = EOL;
	} else {
	    varnam[0] = EOL;
	    tmp[0] = EOL;
	}
	if (varnam[0] == '$') {
	    if (varnam[1] == 't') test = tmp[0];
	    else if (varnam[1] == 'j')
		pid = UNSIGN (tmp[0]) * 256 + UNSIGN (tmp[1]);
	    else if (varnam[1] == 'z' && varnam[2] == 'i') breakon = tmp[0];
	    else {
		stcpy (zref, tmp); nakoffs = UNSIGN (varnam[4]);
	    }
	    continue;
	}
	symtab (i, varnam, tmp);
    }
    newptr = nestnew[nstx];
    nestnew[nstx] = 0;			/* reset pointers */
    return;
#endif
}					/* end unnew() */

void
cleanup ()
{
    set_io (UNIX);			/* reset io_handling */
    lock (" \201", -1, LOCK);		/* un-LOCK */
    lock (" \201", -1, ZDEALLOCATE);	/* ZDEALLOCATE ALL */
    free (buff);			/* free previously allocated space */
    free (svntable);
    free (partition);
    if (apartition)
	free (apartition);
#ifndef NEWSTACK
    free (newstack);
#endif
    if (v22size)
	free (v22ali);
    return;
}					/* end of cleanup */
void
onintr ()
{
    SIGNAL_ACTION(SIGINT, onintr, NULL); /* restore handler */

    if (breakon)
	ierr = INRPT;
    else
	zcc = TRUE;
    return;
}					/* end of onintr */
void
onfpe ()
{
    SIGNAL_ACTION(SIGFPE, onfpe, NULL); /* restore handler */

    ierr = MXNUM;
    return;
}					/* end of onfpe */
void
onquit ()
{
    SIGNAL_ACTION(SIGQUIT, onquit, NULL); /* restore handler */
    if (zbreakon && (ierr == OK))
	ierr = OK - CTRLB;
    return;
}					/* end of onquit */
void
onkill ()
{
#ifdef NEWSTACK
    stack_level *tmp_stack = stack;
#else
    int     n = nstx;
#endif

    SIGNAL_ACTION(SIGTERM, onkill, NULL); /* restore handler */

    if (killerflag == FALSE)
	return;				/* ignore that signal */
/* if there exists an error trap, process as an error */
/* otherwise terminate the job                        */

    if (DSM2err) {			/* DSM V.2 error trapping */
#ifndef NEWSTACK
	if (ztrap[NESTLEVLS + 1][0] != EOL) {
	    ierr = KILLER;
	    return;
	}
#endif
    } else
#ifdef NEWSTACK
        while(tmp_stack) { 
          if(tmp_stack->ztrap[0] != EOL) { ierr = KILLER; return; }
          else { tmp_stack = tmp_stack->previous; }
        }
#else
	while (n >= 0)
	    if (ztrap[n--][0] != EOL) {
		ierr = KILLER;
		return;
	    }
#endif
    cleanup ();
    if (father)
	kill (father, SIGUSR1);		/* advertise death to parent */
    exit (1);				/* terminate mumps */
}					/* end of onkill() */
void
onhup ()
{
#ifdef NEWSTACK
    stack_level *tmp_stack = stack;
#else
    int     n = nstx;
#endif

    SIGNAL_ACTION(SIGHUP, onhup, NULL);       /* restore handler */

    if (huperflag == FALSE)
	return;				/* ignore that signal */
/* if there exists an error trap, process as an error */
/* otherwise terminate the job                        */

    if (DSM2err) {			/* DSM V.2 error trapping */
#ifndef NEWSTACK
	if (ztrap[NESTLEVLS + 1][0] != EOL) {
	    ierr = HUPER;
	    return;
	}
#endif
    } else
#ifdef NEWSTACK
        while(tmp_stack) { 
          if(tmp_stack->ztrap[0] != EOL) { ierr = HUPER; return; }
          else { tmp_stack = tmp_stack->previous; }
        }
#else
	while (n >= 0)
	    if (ztrap[n--][0] != EOL) {
		ierr = HUPER;
		return;
	    }
#endif
    cleanup ();
    if (father)
	kill (father, SIGUSR1);		/* advertise death to parent */
    exit (1);				/* terminate mumps */
}					/* end of onhup() */
void
onbus ()
{
    cleanup ();
    printf ("\012\015BUS ERROR, SEGMENTATION VIOLATION\012\015");
    if (father)
	kill (father, SIGUSR1);		/* advertise death to parent */
    exit (1);				/* terminate mumps */
}					/* end of onbus() */
/* under XENIX processes started with JOB hang around as zombies     */
/* if they HALT before the parent process, unless the parent process */
/* waits for his child to terminate. to solve the problem, the child */
/* sends a signal to his parent to avoid an unattended funeral which */
/* inevitably would result in a living dead sucking up cpu time      */

void
oncld ()
{
    int     status;
					/* ignore signal while as we're here */
    SIGNAL_ACTION(SIGUSR1, SIG_IGN, NULL);

    wait (&status);			/* wait for report from child */

    SIGNAL_ACTION(SIGUSR1, oncld, NULL);/* restore handler */

    return;
}					/* end of oncld() */

/* End of $Source: /cvsroot-fuse/gump/FreeM/src/mumps.c,v $ */
