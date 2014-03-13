/***
 * $Source: /cvsroot-fuse/gump/FreeM/src/mpsdef0.h,v $
 * $Revision: 1.7 $ $Date: 2000/02/29 14:10:24 $
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
 * common constants definitions for all mumps modules
 * 
 */
#ifndef	MDC_VENDOR_ID
#define MDC_VENDOR_ID "49\201"
#endif/*MDC_VENDOR_ID*/

#define byte char
#define toggle(A) (A^=01)

/* if you do not want to run under SCO-UNIX, put the following in comment */
#define SCO

/* if you want mumps to run under XENIX, put the following in comment */
#define SYSFIVE

/* if you want mumps to run under LINUX, un-comment the following */
#define LINUX

/* rlf 01/15/99 If you want to compile under libc-6 (GLIBC), as on */
/*              RedHat Linux >= 5.0, define LINUX_GLIBC.           */
#define LINUX_GLIBC

/* spz 4/19/99 If you want to compile under FreeBSD 2.2.8+, define the */
/* following line... */
/*#define FREEBSD */

/* spz 5/24/99 use the new variable stack */
#define NEWSTACK  
/* #define DEBUG_NEWSTACK */

/* rlf 01/16/99 If you want German error messages, define EM_GERMAN   */
/*              If you want English error messages, define EM_ENGLISH */
#define EM_ENGLISH

/* lmv 1999-05-09 define this if you want to use gmtoff timezone data in */
/*                struct tm instead of tzadj                             */
#define USE_GMTOFF

/* lmv 1999-05-09 define this if you want to include sys/time.h */
#define USE_SYS_TIME_H

/* lmv 1999-05-09 define this if you want to use the gettimeofday function */
/*                instead of the ftime function                            */
#define USE_GETTIMEOFDAY

/* version number */
#include "version.h"
#ifndef	FREEM_VERSION_ID
#define FREEM_VERSION_ID "0.?.0\201"
#endif/*FREEM_VERSION_ID*/


/* if you do not want MUMPS to support color terminals, put this in comment */
#ifdef SCO
#define COLOR
#endif /* SCO */

/* if you do not want MUMPS to support automatic DEM <--> EUR conversion, */
/* put this in comment */
#define EUR2DEM "1.95583\201"

/* if you do not want mumps to run under XENIX or UNIX prior to 5.22, */
/* put the following in comment                                       */
/* #define OLDUNIX      */
#ifdef  OLDUNIX
#define USE_SIGNAL
#define SIGNAL_ACTION(sig,action,what)\
		signal(sig,action)
#else /*OLDUNIX*/
#define USE_SIGACTION
#define SIGNAL_ACTION(sig,action,what)\
		act.sa_handler = action;\
		sigaction(sig, &act, what);
#endif/*OLDUNIX*/


/* end of line symbol  0201/-127 */
#define EOL ((char)'\201')

/* end of line symbol  0202/-126/130 */
#define DELIM ((char)'\202')

/* default size of a 'partition' i.e. intermediate result stack+local vars */
#define DEFPSIZE 10000

/* default size of 'userdefined special variable table' */
#define DEFUDFSVSIZ 1000

/* default size of the NEW stack */
#define DEFNSIZE 4096

/* default number & size of alternate routine buffers */
/* maximum number of routine buffers */
#define MAXNO_OF_RBUF 64
#define DEFNO_OF_RBUF 8
#define DEFPSIZE0 10001

/* number of global files concurrently open */
#define NO_GLOBLS 6

/* length of global blocks */
#define BLOCKLEN 1024

/* number of global buffers */
#define NO_OF_GBUF 6

/* number of DO_FOR_XECUTE levels; i.e. depth of gosub-stack */
#define NESTLEVLS 80

/* depth of parser stack; each pending operation/argument requires one entry */
#define PARDEPTH 128

/* pattern match stack; maximum number of pattern atoms */
#define PATDEPTH 14

/* trace limit in globals module, i.e. trees wo'nt grow to the sky */
#define TRLIM 32

/* arguments in an ESC_sequence */
#define ARGS_IN_ESC 5

/* maximum length of a string, 0 <= $L() <= 255 */
#define STRLEN 65535

/* length of $ZTRAP variable */
#define ZTLEN 20

/* length of $ZF (function key) variable */
#define FUNLEN 128

/* length of $V(3)...$V(8) i.e. path names */
#define PATHLEN 120

/* length of error message */
#define ERRLEN 40

/* number of DATE types */
#define NO_DATETYPE 8
#define NO_TIMETYPE 2
#define MONTH_LEN 10

/* number of zkey() production rules */
#define NO_V93 8

/* depth of CS/CRST (cursor save/restore) stack */
#define CSLEN 1

/* HOME device: number of lines and columns */
#define N_LINES 24
#define N_COLUMNS 80

#define FALSE   0
#define TRUE    1

#define DISABLE 0
#define ENABLE  1

/* parameters for set_io() */
#define UNIX    0
#define MUMPS   1

/* error codes */
#define CTRLB     128
#define OK        0
#define INRPT     1
#define BKERR     2
#define NOSTAND   3
#define UNDEF     4
#define LBLUNDEF  5
#define MISSOPD   6
#define MISSOP    7
#define ILLOP     8
#define QUOTER    9
#define COMMAER  10
#define ASSIGNER 11
#define ARGER    12
#define SPACER   13
#define BRAER    14
#define LVLERR   15
#define DIVER    16
#define ILLFUN   17
#define FUNARG   18
#define ZTERR    19
#define NEXTER   20
#define SELER    21
#define CMMND    22
#define ARGLIST  23
#define INVEXPR  24
#define INVREF   25
#define MXSTR    26
#define TOOPARA  27
#define NOPEN    28
#define NODEVICE 29
#define PROTECT  30
#define GLOBER   31
#define FILERR   32
#define PGMOV    33
#define STKOV    34
#define STORE    35
#define NOREAD   36
#define NOWRITE  37
#define NOPGM    38
#define NAKED    39
#define SBSCR    40
#define ISYNTX   41
#define DBDGD    42
#define KILLER   43
#define HUPER    44
#define MXNUM    45
#define NOVAL    46
#define TYPEMISMATCH 47
#define MEMOV    48	
/* MAXERR = maximum error number plus one */
#define MAXERR  49

/* HOME = default device */
#define HOME 0
/* number of devices/units */
#define MAXDEV 4

/* if tab_clear TBC (CSI 3g) and tab_set HTS (ESC H) are not processed */
/* by the 'HOME' device, define the symbol PROC_TAB to emulate them    */
/* #define PROC_TAB          #otherwise make it comment !!!            */
#define PROC_TAB

/* ASCII control character mnemonics */
#define NUL 0
#define SOH 1
#define STX 2
#define ETX 3
#define EOT 4
#define ENQ 5
#define ACK 6
#define BEL 7
#define BS 8
#define TAB 9
#define LF 10
#define VT 11
#define FF 12
#define CR 13
#define SO 14
#define SI 15
#define DLE 16
#define DC1 17
#define DC2 18
#define DC3 19
#define DC4 20
#define NAK 21
#define SYN 22
#define ETB 23
#define CAN 24
#define EM 25
#define SUB 26
#define ESC 27
#define FS 28
#define GS 29
#define RS 30
#define US 31
#define SP 32
#define DEL 127

/* function select in expr evaluator */
#define STRING 0
#define NAME 1
#define LABEL 2
#define OFFSET 3
#define ARGIND 4

/* function select in global/local variables management */
/* even numbers require 'read/write' access, odd numbers 'read' access */
#define set_sym  0
#define kill_sym 2
#define kill_all 4
#define killexcl 6
#define new_sym  8
#define new_all 10
#define newexcl 12
#define killone 14
#define merge_sym 16
#ifdef NEWSTACK
#define pop_sym	18 /* Pop one variable off stack */
#define megapop	20 /* Pop entire SymTable back */
#endif

#define get_sym  1
#define dat      3
#define order    5
#define query    7
#define bigquery 9
#define getinc  11
#define getnext 13
#define m_alias 15
#define zdata   17

#ifdef NEWSTACK
#define STACK_KILL	0 /* Destroy a variable */
#define STACK_POP	1 /* POP a NEWd variable */
#define STACK_NUKE	2 /* Fry the entire symbol table */
#define STACK_SET	3 /* Set a variable */
#define STACK_POPALL	4 /* Restore a NEW-ALL'd symbol table */

typedef struct _stack_command { 
  char command;         /* kill, pop */
  char *key;
  char *data;
  struct _stack_command *next;  /* Next command on this level */
} stack_cmd;
 
typedef struct _stack_level {
  struct _stack_command *new;      /* Variable Stack commands */
  short command; /* Last command exec'd on this stack (nestc) */
  char *cmdptr;  /* Saved Pointer to last command     (nestp) */
  long  roucur;  /* Saved Routine pointer             (nestr) */
  short level;   /* Saved Level                      (nestlt) */
  char *rouname; /* Saved Routine Name                (nestn) */
  short brk;     /* Saved Break			   (breakstk) */
  char  ztrap[ZTLEN]; /* $ZTRAP 		      (ztrap) */
  char  dsm2err; /* ZTrap DSM2ERR */ 
  
  struct _stack_level *previous;
} stack_level;
extern stack_level *stack;
#endif /* NEWSTACK */

/* sets 8th bit in A */
#define SETBIT(A) ((A)|0200)
/* needed if byte data are to be interpreted as unsigned integer */

#define UNSIGN(A) ((A)&0377)

/* device control for terminal I/O */

#define ECHOON      (~DSW&BIT0)
#define DELMODE     (DSW&BIT2)
#define ESCSEQPROC  (DSW&BIT6)
#define CONVUPPER   (DSW&BIT14)
#define DELEMPTY    (DSW&BIT19)
#define NOCTRLS     (DSW&BIT20)
#define CTRLOPROC   (DSW&BIT21)
#define NOTYPEAHEAD (DSW&BIT25)

#define BIT0  1
#define BIT1  2
#define BIT2  4
#define BIT3  8
#define BIT4  16
#define BIT5  32
#define BIT6  64
#define BIT7  128
#define BIT8  256
#define BIT9  512
#define BIT10 1024
#define BIT11 2048
#define BIT12 4096
#define BIT13 8192
#define BIT14 16384
#define BIT15 32768
#define BIT16 65536
#define BIT17 131072
#define BIT18 262144
#define BIT19 524288
#define BIT20 1048576
#define BIT21 2097152
#define BIT22 4194304
#define BIT23 8388608
#define BIT24 16777216
#define BIT25 33554432
#define BIT26 67108864
#define BIT27 134217728
#define BIT28 268435456
#define BIT29 536870912
#define BIT30 1073741824
#define BIT31 2147483648

/* functions from mumps.c */
void    unnew ();
void    cleanup ();
void    onintr ();
void    onquit ();
void    onkill ();
void    onhup ();
void    onbus ();
void    oncld ();
void    onfpe ();

/* functions from expr.c */
void    expr ();
void    zsyntax ();
void    zdate ();
void    ztime ();
void    zkey ();
void    ssvn ();

/* functions from symtab.c */
void    symtab ();
short int collate ();
short int numeric ();
short int comp ();
void    intstr ();
void    lintstr ();
void    udfsvn ();
long    getpmore ();
long    getumore ();
long    getrmore ();
short int getnewmore ();

/* functions from global.c */
void global ();
void close_all_globals ();

/* functions from operator.c */
short int pattern ();
void    pminmax ();
void    add ();
void    mul ();
void    div ();
void    power ();
void    g_sqrt ();
int     numlit ();
long    intexpr ();
short int tvexpr ();
void    m_op ();

/* functions from service.c */
long int find ();
short int kill_ok ();
void    lineref ();
void    zi ();
void    write_f ();
void    writeHOME ();
void    m_output ();
void    write_m ();
void    write_t ();
void    ontimo ();
void    read_m ();
void    hardcpf ();
int     rdchk0 ();
int     locking ();
void    set_io ();
void    set_break ();
void    set_zbreak ();
void    zload ();
void    zsave ();
void    lock ();
void    getraddress ();

/* functions from compr.c */
void    CompressString ();
void    ExpandString ();

/* functions from strings.c */
long int stlen ();
long int stcpy ();
void    stcpy0 ();
void    stcpy1 ();
short int stcat ();
short int stcmp ();

/* CRT screen */
struct vtstyp {
    unsigned char **screenx;	/* characters */
    unsigned char **screena;	/* attributes */
#ifdef COLOR
    unsigned char **screenc;	/* colors     */
#endif					/* COLOR */
  char    *sclines;	/* lines translation table   */
    char    rollflag;			/* Roll or Page mode */
    char    lin24;			/* 24 lines or 25 lines mode */
    char    savarg;
    char    *tabs;
    unsigned char Xpos;
    unsigned char Ypos;
    unsigned char sc_up;
    unsigned char sc_lo;
    unsigned char csx[CSLEN];
    unsigned char csy[CSLEN];
    short   cssgr[CSLEN];		/* save SGR flag */
    short   cscol[CSLEN];		/* save SGR flag */
    short   cs;
    unsigned char att;
#ifdef COLOR
    unsigned char col;			/* color byte */
#endif					/* COLOR */
#ifdef SCO
    unsigned char bw;			/* black_on_white flag */
#endif					/* SCO */
};

/* functions from views.c */
void    view_com ();
void    view_fun ();
short int newpsize ();
short int newusize ();
short int newrsize ();
void    zreplace ();
short int tstglvn ();
void    zname ();
short int znamenumeric ();
void    procv22 ();
void    v25 ();
void    m_tolower ();
void    part_ref ();

/* End of $Source: /cvsroot-fuse/gump/FreeM/src/mpsdef0.h,v $ */
