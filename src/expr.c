/***
 * $Source: /cvsroot-fuse/gump/FreeM/src/expr.c,v $
 * $Revision: 1.7 $ $Date: 2000/02/22 17:48:23 $
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
 * expression evaluator
 * 
 */

#include <sys/types.h>
#include <sys/timeb.h>

/* mumps expression evaluator */

#include "mpsdef.h"

#ifdef USE_SYS_TIME_H
#include <sys/time.h>
#endif

#define OPERAND       1
#define ARRAY         2
#define FNUMBER       3
#define REVERSE       4
#define TRANSLATE     5
#define QLENGTH       6
#define QSUBSCRIPT    7

#define ZCRC          8
#define ZDATA         9
#define ZHTIME       10
#define ZLSD         11
#define ZNEXT        12
#define ZPREVIOUS    17
#define ZTRAP        18

#define SVNsystem    19
#define SVNtimezone  20
#define SVNtlevel    22
#define SVNtrollback 23
#define SVNecode     24
#define SVNestack    25
#define SVNetrap     26
#define SVNstack     27

#define OR                '!'
#define MODULO            '#'
#define DIVIDE            '/'
#define AND               '&'
#define NOT               '\''
#define XOR		  '~'
#define MULTIPLY          '*'
#define POWER             ' '
#define PLUS              '+'
#define MINUS             '-'
#define LESS              '<'
#define EQUAL             '='
#define GREATER           '>'
#define PATTERN           '?'
#define INDIRECT          '@'
#define CONTAINS          '['
#define INTDIVIDE         '\\'
#define FOLLOWS           ']'
#define CONCATENATE       '_'
#define SORTSAFTER        '.'
#define EQFOLLOWS         ','
#define EQSORTS           ';'
#define MAXOP             ':'
#define MINOP             '%'

#define GET               'Y'
#define GETX              ':'

long    time ();
char   *calloc ();
void    cond_round ();
void    ssvn ();
void    zdate ();
void    zkey ();
void    ztime ();
int     levenshtein ();

void
expr (extyp)
	short   extyp;			/* type of expression: */

					/* STRING,NAME,LABEL,OFFSET,ARGIND */

{
    char    op_stck[PARDEPTH + 1];	/* operator/operandflag stack */
    short   spx;			/* stack pointer:             */
    short   zexflag;			/* z 'intrinsic' function flag */
    int     atyp,
            btyp;			/* DM/EUR currency types */
    char   *a;				/* pointer to current (left) argument */
    char   *b;				/* pointer to right hand argument     */
    char    tmp[256];
    int     refsx;			/* zref/zloc stack_counter  */
    char   *refsav[PARDEPTH];		/* zref/zloc stack          */


    register i,
            j,
            f,
            ch;
    short   group;			/* flag to scan grouped patterns */
#ifdef DEBUG_NEWPTR
    int	loop;
#endif
#ifdef NEWSTACK
    stack_level *tmp_stack;
#endif

    refsx = 0;

    if (extyp == NAME) {
	f = *codptr;
	varnam[0] = f;
	if ((f >= 'A' && f <= 'Z') ||
	    (f >= 'a' && f <= 'z') ||
	    f == '^' || f == '$' || f == '%') {
	    i = 1;
	    while (((ch = *++codptr) >= 'A' && ch <= 'Z') ||
		   (ch >= 'a' && ch <= 'z') ||
		   (ch >= '0' && ch <= '9' && (i > 1 || f != '^')) ||
		   f == '^' &&
		   (((ch == '%' || ch == '$') && i == 1) ||
		    (standard == 0 &&
		     (ch == '.' ||
		      (ch == '/' && i == 1) ||
		      (((ch == '/' && varnam[i - 1] != '/') ||
			(ch == '%' && varnam[i - 1] == '/')) &&
		       (varnam[1] == '.' || varnam[1] == '/')))))) {
		varnam[i++] = ch;
	    }

	    varnam[i] = EOL;
	    if (ch == '(') {		/* it's an array */
		op_stck[0] = 0;
		op_stck[1] = ARRAY;
		spx = 1;
/*           argstck[0]= */ a = argptr;
		if ((argstck[1] = a) >= s) {
		    char   *bak;

		    bak = partition;
		    if (getpmore == 0) {
			ierr = STKOV;
			return;
		    }
		    a = a - bak + partition;
		    b = b - bak + partition;
		}
		a += stcpy (a, varnam) + 1;
		arg = 1;
		codptr++;
		goto nextchr;
	    }
	    codptr--;
	    if (i == 1 && f == '^') {
		ierr = INVEXPR;
	    }
	    return;
	}
	if (f != '@') {
	    ierr = INVREF;
	    return;
	}
    }					/* end if (extyp ==NAME) */
    arg = 0;
    spx = 0;				/* initialisation */
    op_stck[0] = 0;
/*      argstck[0]= */ a = argptr;

  nextchr:
    ch = *codptr;
    if ((ch >= 'A' && ch <= 'Z') ||
	(ch >= 'a' && ch <= 'z') || ch == '%') {

      scan_name:

	varnam[0] = ch;
	i = 1;
	if (ch == '^') {		/* global variable name */
	    while (((ch = *++codptr) >= 'A' && ch <= 'Z') ||
		   (ch >= 'a' && ch <= 'z') ||
		   (ch >= '0' && ch <= '9' && i > 1) ||
		   (((ch == '%' || ch == '$') && i == 1) ||
		    (standard == 0 &&
		     (ch == '.' ||
		      (ch == '/' && i == 1) ||
		      (((ch == '/' && varnam[i - 1] != '/') ||
			(ch == '%' && varnam[i - 1] == '/')) &&
		       (varnam[1] == '.' || varnam[1] == '/')))))) {
		varnam[i++] = ch;
	    }
	    varnam[i] = EOL;
	    if (i == 1 && ch != '(') {
		ierr = INVEXPR;
		return;
	    }
	} else {			/* local variable name */
	    while (((ch = *++codptr) >= 'A' && ch <= 'Z') ||
		   (ch >= 'a' && ch <= 'z') ||
		   (ch >= '0' && ch <= '9')) {
		varnam[i++] = ch;
	    }
	    varnam[i] = EOL;
	}
	if (ch == '(') {		/* it's an array */
	    if (extyp == LABEL) {
		codptr--;
		return;
	    }
	    if (++spx >= PARDEPTH) {
		ierr = STKOV;
		return;
	    }
	    op_stck[spx] = ARRAY;
	    if ((argstck[++arg] = a) >= s) {
		char   *bak;

		bak = partition;
		if (getpmore == 0) {
		    ierr = STKOV;
		    return;
		}
		a = a - bak + partition;
		b = b - bak + partition;
	    }
	    a += stcpy (a, varnam) + 1;
	    codptr++;
	    goto nextchr;
	}
	if (spx == 0) {
	    if (extyp != STRING && extyp != ARGIND && extyp != OFFSET) {
		codptr--;
		return;
	    }
	    if (varnam[0] != '^')
		symtab (get_sym, varnam, a);
	    else if (varnam[1] != '$')
		global  (get_sym, varnam, a);

	    else
		ssvn (get_sym, varnam, a);
	    if (ierr != OK) {
		stcpy (varerr, varnam);
		if (ierr == UNDEF || ierr == (UNDEF - CTRLB)) {
		    arg = 1;
		    codptr--;
		    goto undefglvn;
		}
	    }
	    if (ch == EOL || ch == SP ||
		(extyp == ARGIND) ||
		ch == ',' || ch == ':' || ch == ')' || ch == '@' ||
		(ierr > OK))
		return;
	    arg = 1;
	    argstck[1] = a;
	    f = OPERAND;
	    op_stck[1] = f;
	    spx = 2;
	    goto op10;			/* shortcut: following char is garbage or operator */
	}
	codptr--;
	if ((argstck[++arg] = a) >= s) {
	    char   *bak;

	    bak = partition;
	    if (getpmore == 0) {
		ierr = STKOV;
		return;
	    }
	    a = a - bak + partition;
	    b = b - bak + partition;
	}
/* evaluate glvn or $_(glvn) */

      var1:if (op_stck[spx] == '$') {
	    f = op_stck[spx - 1];
	    switch (f) {
	    case 'd':			/* $DATA */
		ch = dat;

	      glv_fcn:
		if (varnam[0] != '^')
		    symtab (ch, varnam, a);
		else if (varnam[1] != '$')
		    global  (ch, varnam, a);

		else
		    ssvn (ch, varnam, a);
	      d_o_n:
		if (*++codptr != ')')
		    ierr = INVEXPR;
		if (ierr > OK) {
		    stcpy (varerr, varnam);
		    return;
		}
		spx -= 2;
		goto nxt_operator;

	    case 'o':			/* $ORDER */
		ch = order;
		ordercnt = 1L;
		if (*(codptr + 1) != ',') {
		    ordercounter = 0;
		    goto glv_fcn;
		}
		if (++spx > PARDEPTH) {
		    ierr = STKOV;
		    return;
		}
		stcpy (a, varnam);
		op_stck[spx] = OPERAND;
		codptr++;
		goto nextchr;

	    case 'n':			/* $NEXT */

		ordercnt = 1L;
		ordercounter = 0;
		if (varnam[0] != '^')
		    symtab (order, varnam, a);
		else if (varnam[1] != '$')
		    global  (order, varnam, a);

		else
		    ssvn (order, varnam, a);

		if (a[0] == EOL) {
		    a[0] = '-';
		    a[1] = '1';
		    a[2] = EOL;
		}
		goto d_o_n;

	    case 'q':			/* $QUERY */
	    case 'O':			/* $ZORDER */
		ch = query;
		ordercnt = 1L;
		if (*(codptr + 1) != ',')
		    goto glv_fcn;
		if (++spx > PARDEPTH) {
		    ierr = STKOV;
		    return;
		}
		stcpy (a, varnam);
		op_stck[spx] = OPERAND;
		codptr++;
		goto nextchr;

	    case ZNEXT:		/* $ZNEXT */
		ordercnt = 1L;
		if (varnam[0] != '^')
		    symtab (query, varnam, a);
		else if (varnam[1] != '$')
		    global  (query, varnam, a);

		else
		    ssvn (query, varnam, a);
		if (a[0] == EOL) {
		    a[0] = '-';
		    a[1] = '1';
		    a[2] = EOL;
		}
		goto d_o_n;

	    case 'N':			/* $NAME */
/* resolve naked reference */
		if (varnam[0] == '^' && varnam[1] == DELIM) {
		    stcpy (a, zref);
		    ch = stlen (a);
		    while (a[ch--] != DELIM)
			if (ch <= 0) {
			    ierr = NAKED;
			    return;
			}
		    stcpy (&a[++ch], &varnam[1]);
		    stcpy (varnam, a);
		}
		if (*(codptr + 1) != ',') {
		    zname (a, varnam);
		    goto d_o_n;
		}
		if (++spx > PARDEPTH) {
		    ierr = STKOV;
		    return;
		}
		stcpy (a, varnam);
		op_stck[spx] = OPERAND;
		codptr++;
		goto nextchr;

	    case ZPREVIOUS:		/* $ZPREVIOUS */
		ordercnt = (-1L);
		ordercounter = 0;
		ch = order;
		goto glv_fcn;

	    case ZDATA:		/* $ZDATA */
		ch = zdata;
		goto glv_fcn;

	    case 'g':			/* $GET */
		if (varnam[0] != '^')
		    symtab (get_sym, varnam, a);
		else if (varnam[1] != '$')
		    global  (get_sym, varnam, a);

		else
		    ssvn (get_sym, varnam, a);

		if (ierr > OK) {
		    stcpy (varerr, varnam);
		    if (ierr != UNDEF && ierr != (UNDEF - CTRLB))
			return;
		}
		if (ierr == UNDEF || ierr == (UNDEF - CTRLB)) {
		    ierr = ierr < 0 ? OK - CTRLB : OK;
		    if (*++codptr == ',') {
			if (standard) {
			    ierr = NOSTAND;
			    return;
			}
			op_stck[spx - 1] = GET;		/* dummy function for $GET */
			arg--;
			codptr++;
			goto nextchr;
		    } else {
			if (*codptr != ')') {
			    ierr = INVEXPR;
			    return;
			}
			*a = EOL;
		    }
		} else {		/* glvn was defined */
		    if (*++codptr == ',') {	/* skip second argument */
			i = 0;		/* quote flag */
			f = 0;		/* bracket counter */
			for (;;)
			{
			    ch = *++codptr;
			    if (ch == EOL) {
				ierr = INVEXPR;
				return;
			    }
			    if (ch == '"') {
				i = !i;
				continue;
			    }
			    if (i)
				continue;
			    if (ch == '(') {
				f++;
				continue;
			    }
			    if (ch == ')') {
				if (--f < 0)
				    break;
			    }
			}
		    } else if (*codptr != ')') {
			ierr = INVEXPR;
			return;
		    }
		}
		spx -= 2;
		goto nxt_operator;

	    case 'i':			/* $INCREMENT */
		if (varnam[0] != '^')
		    symtab (getinc, varnam, a);
		else {
		    int     setopsav;

		    setopsav = setop;
		    setop = '+';
		    a[0] = '1';
		    a[1] = EOL;
		    if (varnam[1] != '$')
			global  (set_sym, varnam, a);

		    else
			ssvn (set_sym, varnam, a);
		    setop = setopsav;
		}
		goto d_o_n;


	    case OPERAND:		/* three arguments $TEXT */

		if (spx >= 6 &&
		    op_stck[spx - 5] == 't' &&
		    op_stck[spx - 4] == '$' &&
		    op_stck[spx - 2] == '$') {
		    stcpy (a, &varnam[varnam[0]=='^']); /* third argument */
		    if (++spx > PARDEPTH) {
			ierr = STKOV;
			return;
		    }
		    op_stck[spx] = OPERAND;
		    codptr++;
		    goto nextchr;
		}
	    }				/* end switch */
	}
/* retrieve look-up */

	if (varnam[0] != '^')
	    symtab (get_sym, varnam, a);
	else if (varnam[1] != '$')
	    global  (get_sym, varnam, a);

	else
	    ssvn (get_sym, varnam, a);
      undefglvn:;
	if (ierr)
	    stcpy (varerr, varnam);
	if (ierr == UNDEF || ierr == (UNDEF - CTRLB)) {
	    stcpy (tmp, codptr + 1);
	    if (varnam[0] == '^') {	/* is there a default expression?? */
		if (gvndefault[0] == EOL)
		    return;
		stcpy (&code[1], gvndefault);
	    } else {
		if (lvndefault[0] == EOL)
		    return;
		stcpy (&code[1], lvndefault);
	    }
/* simulate a $GET function */
	    code[0] = SP;
	    stcat (code, ")\201");
	    stcat (code, tmp);
	    codptr = &code[1];
	    if (((++spx) + 1) > PARDEPTH) {
		ierr = STKOV;
		return;
	    }
	    op_stck[spx] = GETX;	/* dummy function for $GET */
	    op_stck[++spx] = '$';
/* stack $ZREFERENCE and $ZLOCAL */
	    if ((refsav[refsx] = calloc (1, 2 * 256)) == NULL) {
		ierr = STKOV;
		return;
	    }				/* could not allocate stuff...     */
	    stcpy (refsav[refsx], zref);
	    stcpy (refsav[refsx++] + 256, zloc);
	    ierr -= UNDEF;
	    arg--;
	    goto nextchr;
	}
	if (ierr > OK)
	    return;

	if (spx == 0) {
	    if ((ch = *++codptr) == EOL || ch == SP ||
		ch == ',' || ch == ':')
		return;
	    if (++spx > PARDEPTH) {
		ierr = STKOV;
		return;
	    }
	    op_stck[spx] = OPERAND;
	    goto next10;
	}
	f = op_stck[spx];
	if (f == ARRAY || f == '(') {
	    if (++spx > PARDEPTH) {
		ierr = STKOV;
		return;
	    }
	    op_stck[spx] = OPERAND;
	    codptr++;
	    goto nextchr;
	}
	if (f == INDIRECT && (extyp == STRING || extyp == ARGIND || extyp == OFFSET)) {
	    spx--;
	    goto indirect;		/* VARIABLE indirection */
	}
	goto nxt_expr;

    }
    if (ch >= '0' && ch <= '9') {
	if (extyp == LABEL)
	    goto scan_name;		/* scan_label */

/* scan number */
	i = 0;				/* point flag */
	j = 0;				/* exp flag */
	f = ch;				/* first character */

	if ((argstck[++arg] = a) >= s) {
	    char   *bak;

	    bak = partition;
	    if (getpmore == 0) {
		ierr = STKOV;
		return;
	    }
	    a = a - bak + partition;
	}
	b = a;

      p_entry:;			/* entry point if first character was a point */
	for (;;)
	{
	    if (ch < '0') {
		if (ch != '.' || i || j)
		    break;
		i++;
	    } else if (ch > '9') {
		if (j)
		    break;
		if (ch != 'E' && (lowerflag == FALSE || ch != 'e'))
		    break;
		if (ch == 'E') {
		    if ((*(codptr + 1) == 'U') && (*(codptr + 2) == 'R'))
			break;
		    if ((*(codptr + 1) == 'S') && (*(codptr + 2) == 'P'))
			break;
		}
		j++;
		do {
		    *b++ = ch;
		    ch = *++codptr;
		} while (ch == '+' || ch == '-');
	    }
	    *b++ = ch;
	    ch = *++codptr;
	}
#ifdef EUR2DEM
	switch (ch) {
	case 'E':
	    if ((*(codptr + 1) == 'U') && (*(codptr + 2) == 'R')) {
		*b++ = ch;
		*b++ = *++codptr;
		*b++ = *++codptr;
		ch = *++codptr;
		j = 1;
		break;
	    }
	    if ((*(codptr + 1) == 'S') && (*(codptr + 2) == 'P')) {
		*b++ = ch;
		*b++ = *++codptr;
		*b++ = *++codptr;
		ch = *++codptr;
		j = 1;
	    }
	    break;
	case 'D':
	    if (*(codptr + 1) == 'M') {
		*b++ = ch;
		*b++ = *++codptr;
		ch = *++codptr;
		j = 1;
		break;
	    }
	    if (*(codptr + 1) == 'E' && *(codptr + 2) == 'M') {
		*b++ = ch;
		*b++ = *++codptr;
		*b++ = *++codptr;
		ch = *++codptr;
		j = 1;
	    }
	    break;
	case 'A':
	    if (*(codptr + 1) == 'T' && *(codptr + 2) == 'S') {
		*b++ = ch;
		*b++ = *++codptr;
		*b++ = *++codptr;
		ch = *++codptr;
		j = 1;
	    }
	    break;
	case 'B':
	    if (*(codptr + 1) == 'F' && *(codptr + 2) == 'R') {
		*b++ = ch;
		*b++ = *++codptr;
		*b++ = *++codptr;
		ch = *++codptr;
		j = 1;
	    }
	    break;
	case 'F':
	    if (*(codptr + 1) == 'F') {
		*b++ = ch;
		*b++ = *++codptr;
		ch = *++codptr;
		j = 1;
		break;
	    }
	    if (*(codptr + 1) == 'M' && *(codptr + 2) == 'K') {
		*b++ = ch;
		*b++ = *++codptr;
		*b++ = *++codptr;
		ch = *++codptr;
		j = 1;
		break;
	    }
	    if (*(codptr + 1) == 'R' && *(codptr + 2) == 'F') {
		*b++ = ch;
		*b++ = *++codptr;
		*b++ = *++codptr;
		ch = *++codptr;
		j = 1;
	    }
	    break;
	case 'I':
	    if (*(codptr + 1) == 'E' && *(codptr + 2) == 'P') {
		*b++ = ch;
		*b++ = *++codptr;
		*b++ = *++codptr;
		ch = *++codptr;
		j = 1;
		break;
	    }
	    if (*(codptr + 1) == 'T' && *(codptr + 2) == 'L') {
		*b++ = ch;
		*b++ = *++codptr;
		*b++ = *++codptr;
		ch = *++codptr;
		j = 1;
	    }
	    break;
	case 'N':
	    if (*(codptr + 1) == 'L' && *(codptr + 2) == 'G') {
		*b++ = ch;
		*b++ = *++codptr;
		*b++ = *++codptr;
		ch = *++codptr;
		j = 1;
	    }
	    break;
	case 'P':
	    if (*(codptr + 1) == 'T' && *(codptr + 2) == 'E') {
		*b++ = ch;
		*b++ = *++codptr;
		*b++ = *++codptr;
		ch = *++codptr;
		j = 1;
	    }
	}
#endif /* EUR2DEM */
	*b = EOL;
	if (j || f == '0' || (i && ((*(b - 1)) < '1'))) {	/* <'1' eqiv. to '.' || '0' */
	    atyp = numlit (a);
	    if (atyp)
		stcat (a, WHR[atyp]);
	}
	if (spx) {
	    codptr--;
	    goto exec;
	}
	if (ch == EOL || ch == SP ||
	    ch == ',' || ch == ':' ||
	    (ch == '^' && extyp == OFFSET))
	    return;
	spx = 1;
	op_stck[1] = OPERAND;
    }
    if (ch != '"')
	goto next10;
/* scan string */
    if ((argstck[++arg] = a) >= s) {
	char   *bak;

	bak = partition;
	if (getpmore == 0) {
	    ierr = STKOV;
	    return;
	}
	a = a - bak + partition;
	b = b - bak + partition;
    }
    i = 0;
    for (;;)
    {
	while ((ch = *++codptr) > '"') {
	    a[i++] = ch;
	}
/* we make use of the fact that */
/* EOL < "any ASCII character" */
	if (ch == '"' && (ch = *++codptr) != '"') {
	    if (ch == '_' && *(codptr + 1) == '"') {
		codptr++;
		continue;
	    }
	    a[i] = EOL;
	    if (spx) {
		codptr--;
		goto exec;
	    }
	    if (ch == EOL || ch == SP ||
		ch == ',' || ch == ':')
		return;
	    spx = 1;
	    op_stck[1] = OPERAND;
	    goto next10;
	}
	if (ch == EOL) {
	    ierr = QUOTER;
	    return;
	}
	a[i++] = ch;
    }
  next05:
    ch = *(++codptr);
  next10:
    switch (ch) {
    case EOL:
    case SP:
	if (op_stck[1] == OPERAND && spx == 1)
	    return;
	ierr = INVEXPR;
	return;

    case ',':
	if (spx == 0) {
	    ierr = ARGER;
	    return;
	}
      comma:
	f = op_stck[spx - 1];
/* f= (spx>0 ? op_stck[spx-1] : 0);
 * if (f) */ switch (f) {
	case '$':			/* first arg of $function */
	    if (op_stck[spx - 2] == 's') {	/* we already have one valid arg */
		i = 0;			/* quote *//* and skip rest of select */
		j = 0;			/* bracket */
		for (;;)
		{
		    ch = *++codptr;
		    if (ch == '"') {
			toggle (i);
			continue;
		    }
		    if (i) {
			if (ch != EOL)
			    continue;
			ierr = QUOTER;
			return;
		    }
		    if (ch == ')') {
			if (j--)
			    continue;
			spx -= 3;
			goto nxt_operator;
		    }
		    if (ch == '(') {
			j++;
			continue;
		    }
		    if (ch == EOL) {
			ierr = SELER;
			return;
		    }
		}

	    }
/* function argument */
/* put comma on the stack */
	    if (++spx > PARDEPTH) {
		ierr = STKOV;
		return;
	    }
	    op_stck[spx] = f;		/* '$' */
/*       a+=stlen(a)+1; */ while (*a++ != EOL) ;
	    codptr++;
	    goto nextchr;

	case ARRAY:			/* array subscript */

	    *(a - 1) = DELIM;
	    arg--;
	    spx--;
/*      a+=stlen(a)+1; */ while (*a++ != EOL) ;
	    codptr++;
	    goto nextchr;

	default:
	    if ((extyp == NAME) || (spx > 1)) {
		ierr = INVEXPR;
		return;
	    }
	    return;
	}

    case '^':
	if (extyp == LABEL || extyp == OFFSET)
	    break;
      uparrow:;
	if (spx >= 5) {			/* take care of $TEXT with three args */
	    if (op_stck[spx - 4] == 't' &&
		op_stck[spx - 3] == '$' &&
		op_stck[spx - 1] == '$') {
		if (++spx > PARDEPTH) {
		    ierr = STKOV;
		    return;
		}
		op_stck[spx] = '$';
		while (*a++ != EOL);
		if (*(codptr+1)=='@') goto next05;
	    }
	}
	goto scan_name;
    case '.':
	if ((ch = *++codptr) < '0' || ch > '9') {
	    ierr = INVEXPR;
	    return;
	}
	if ((argstck[++arg] = a) >= s) {
	    char   *bak;

	    bak = partition;
	    if (getpmore == 0) {
		ierr = STKOV;
		return;
	    }
	    a = a - bak + partition;
	    b = b - bak + partition;
	}
	i = 1;				/* point flag */
	j = 0;				/* exp flag */
	f = '.';			/* first character */
	b = a;
	*b++ = f;
	goto p_entry;

    case ')':
	if (spx <= 1) {
	    if (setpiece)
		return;
	    if (spx == 0) {
		ierr = BRAER;
		return;
	    }
	}
	if (op_stck[spx] != OPERAND) {
	    ierr = INVEXPR;
	    return;
	}
	if ((f = op_stck[spx - 1]) == ARRAY) {	/* array return */
	    *--a = DELIM;
	    stcpy (varnam, a = argstck[--arg]);
	    if ((spx -= 2) <= 0 && extyp != STRING && extyp != ARGIND)
		return;
	    goto var1;
	}
/* precedence close parenthesis */
	if (f == '(') {
	    spx -= 2;
	    goto nxt_operator;
	}
	if (spx <= 2) {
	    ierr = BRAER;
	    return;
	}				/* unmatched ')' */
/**
 * *********** function evaluation ******************************************
 * 
 * Note: Input for function() is found in 'partition':
 * There are 'f' arguments to be found at 'a'
 * The arguments are separated by an EOL character.
 * There is a list of the addresses of the arguments
 * in 'a==argstck[arg], argstck[arg+1], argstck[arg+f-1]'
 * Result is returned at a==argstck[arg]
 * 
 */
	f = 1;				/* f == number of arguments */
	if (op_stck[spx -= 2] == OPERAND) {
	    do {
		f++;
		arg--;
	    } while (op_stck[spx -= 2] == OPERAND);
	    a = argstck[arg];
	}
	i = op_stck[spx--];

	switch (i) {			/* function select */
	case 'e':			/* $EXTRACT */

	    switch (f) {
	    case 1:
		a[1] = EOL;
		goto nxt_operator;
	    case 2:
		b = argstck[arg + 1];
		i = intexpr (b) - 1;	/* numeric value of 2nd argument */
		if (ierr == MXNUM) {
		    ierr = OK;
		    if (i >= 0)
			i = 256;
		}
		f = b - a - 1;		/* length of first argument */
		if (i > f || i < 0) {
		    a[0] = EOL;
		}
/* out of range */
		else {
		    a[0] = a[i];
		    a[1] = EOL;
		}			/* get character */
		goto nxt_operator;
	    case 3:
/* numeric value of 3rd argument */
		if ((j = intexpr (argstck[arg + 2])) > 256)
		    j = 256;
		if (ierr == MXNUM) {
		    ierr = OK;
		    if (j >= 0)
			j = 256;
		}
/* numeric value of 2nd argument */
		b = argstck[arg + 1];
		if ((i = intexpr (b)) <= 0)
		    i = 1;
		if (ierr == MXNUM) {
		    ierr = OK;
		    if (j >= 0)
			j = 256;
		}
		f = b - a - 1;		/* length of first argument */
/* out of range */
		if (i > f || j < i) {
		    a[0] = EOL;
		    goto nxt_operator;
		}
		if (j < f)
		    a[j] = EOL;
		if (--i > 0)
		    stcpy (a, &a[i]);
		goto nxt_operator;
	    default:
		ierr = FUNARG; {
		    return;
		}
	    }

	case 'a':			/* $ASCII */

	    if (f == 1) {
		intstr (a, (*a != EOL ? UNSIGN ((int) *a) : -1));
		goto nxt_operator;
	    }
	    if (f > 2) {
		ierr = FUNARG;
		return;
	    }
	    b = argstck[arg + 1];
	    i = intexpr (b);
/* ascii number of selected character or -1 if out of range */
	    intstr (a, (i >= (b - a)) || i <= 0 ? -1 : UNSIGN ((int) a[i - 1]));
	    goto nxt_operator;

	case 'c':			/* $CHARACTER */

	    {
		short   l,
		        l1,
		        m,
		        n;

		l1 = f;
		i = 0;
		f = 0;
		j = 0;
		m = 0;
		n = 1;
		l = 0;
		for (;;)
		{
		    if ((ch = a[i++]) == EOL) {
			if (m == 0) {
			    if (j > DEL) {
				if (standard) {
				    ierr = NOSTAND;
				    return;
				}
				if (eightbit) {
				    j &= 0377;
				    if ((((char) j) == EOL) || (((char) j) == DELIM))
					j = NUL;
				} else
				    j &= 0177;
			    }
/*ldl*/if (f > STRLEN) 1/0; /* 'f' should never be > STRLEN */
			    if (f >= STRLEN) {
				a[f] = EOL;
				ierr = MXSTR;
				return;
			    }
			    a[f++] = j;
			}
			if (++l >= l1)
			    break;
			j = 0;
			m = 0;
			n = 1;
			continue;
		    }
		    if (n == 0)
			continue;
		    if (ch >= '0' && ch <= '9') {
			j *= 10;
			j += ch - '0';
			continue;
		    }
		    if (ch == '-') {
			m |= 01;
			continue;
		    }
		    if (ch != '+')
			n = 0;
		}
		a[f] = EOL;
	    }
	    goto nxt_operator;

	case 'p':			/* $PIECE */

	    {
		long    l,
		        l1,
		        m,
		        n;

		b = argstck[arg + 1];
		l1 = b - a - 1;		/* length of 1st argument */

		switch (f) {
		case 2:
		    f = 1;
		    l = 1;
		    break;
		case 3:
		    f = intexpr (argstck[arg + 2]);
		    if (ierr == MXNUM) {
			ierr = OK;
			if (j >= 0)
			    f = 256;
		    }
		    if (f <= 0) {
			a[0] = EOL;
			goto nxt_operator;
		    }
		    l = f;
		    break;
		case 4:
		    l = intexpr (argstck[arg + 3]);
		    if (ierr == MXNUM) {
			ierr = OK;
			if (l >= 0)
			    l = 256;
		    }
		    if ((f = intexpr (argstck[arg + 2])) <= 0)
			f = 1;
		    if (ierr == MXNUM) {
			ierr = OK;
			if (f >= 0)
			    f = 256;
		    }
		    if (f > l) {
			a[0] = EOL;
			goto nxt_operator;
		    }
		    break;
		default:
		    ierr = FUNARG;
		    return;
		}

		i = 0;
		m = 0;
		ch = 0;
		while (b[ch] != EOL)
		    ch++;		/* $l of 2nd arg */

		if (ch == 1) {
		    ch = b[0];
		    j = 1;
		    if (f > 1) {
			while (i < l1) {	/* scan 1st string ... */
			    if (a[i++] != ch)
				continue;	/* ... for occurence of 2nd */
			    if (++j == f) {
				m = i;
				goto p10;
			    }
			}
/* if(j<f) */ a[0] = EOL;
			goto nxt_operator;
		    }
		  p10:for (; i < l1; i++) {
			if (a[i] != ch)
			    continue;
			if (j == l) {
			    a[i] = EOL;
			    break;
			}
			j++;
		    }
		    if (m > 0)
			stcpy (a, &a[m]);
		    goto nxt_operator;
		}
		if (ch == 0) {
		    a[0] = EOL;
		    goto nxt_operator;
		}			/* 2nd arg is empty */
/* else (ch>1) */
		n = 1;
		if (f > 1) {
		    while (i < l1) {	/* scan 1st string ... */
			j = 0;
		      p20:if (a[i + j] != b[j]) {
			    i++;
			    continue;
			}		/* ... for occurence of 2nd */
			if (++j < ch)
			    goto p20;
			i += ch;	/* skip delimiter */
			if (++n == f) {
			    m = i;
			    goto p30;
			}
		    }
/* if(n<f) */ a[0] = EOL;
		    goto nxt_operator;
		}
	      p30:while (i < l1) {
		    j = 0;
		  p40:if (a[i + j] != b[j]) {
			i++;
			continue;
		    }
		    if (++j < ch)
			goto p40;
		    if (n == l) {
			a[i] = EOL;
			break;
		    }			/* last $piece: done! */
		    i += ch;
		    n++;
		}
		if (m > 0)
		    stcpy (a, &a[m]);
		goto nxt_operator;
	    }

	case 'l':			/* $LENGTH */

	    if (f == 1) {
		intstr (a, (short) stlen (a));
		goto nxt_operator;
	    }
	    if (f > 2) {
		ierr = FUNARG;
		return;
	    }
	    i = 0;
	    j = 0;
	    ch = 0;
	    b = argstck[arg + 1];
	    if ((f = stlen (b))) {
		f--;
		while ((i = find (&a[ch], b)) > 0) {
		    j++;
		    ch += i + f;
		}
		j++;
	    }
	    intstr (a, j);
	    goto nxt_operator;

	case 'f':			/* $FIND */

	    {
		short   l1;

		if (f < 2 || f > 3) {
		    ierr = FUNARG;
		    return;
		}
		if (f == 3) {
		    i = intexpr (argstck[arg + 2]);
		    if (ierr == MXNUM) {
			if (i > 0)
			    i = 256;
			ierr = OK;
/* important special case:
 * $FIND("","",number) ::= $S(number<1:1,1:integer(number))
 * needs special treatment so that it does not yield wrong
 * results on large values of number.
 */
			if ((argstck[arg + 1][0] == EOL) && (i > 0)) {
			    numlit (argstck[arg + 2]);
			    i = 0;
			    while ((a[i] = argstck[arg + 2][i]) != EOL) {
				if (a[i] == '.') {
				    a[i] = EOL;
				    break;
				}
				i++;
			    }
			    goto nxt_operator;
			}
		    }
		    i--;
		    if (i < 0)
			i = 0;
		} else
		    i = 0;
		b = argstck[arg + 1];
		j = b - a - 1;		/* length of first argument */
		if ((l1 = stlen (b)) == 0) {
		    i++;
		    goto f20;
		}
		for (f = i; f < j; f++) {
		    for (ch = 0; ch < l1; ch++) {
			if (a[f + ch] != b[ch])
			    goto f10;
		    }
		    i = (++f) + l1;
		    goto f20;
		  f10:;
		}
		i = 0;
	      f20:;
		lintstr (a, i);
	    }
	    goto nxt_operator;

	case 'j':			/* $JUSTIFY */

	    if (f < 2 || f > 3) {
		ierr = FUNARG;
		return;
	    } {
		long    l,
		        l1;

		l = intexpr (b = argstck[arg + 1]);	/* 2nd arg */
		if (ierr == MXNUM) 	/* $J() arg number overflow	*/
		    return;
		if (l > STRLEN) {
					/* $J() width string too long	*/
		    ierr = MXSTR;
		    return;
		}
		if (f == 2) {
		    f = b - a - 1;
		} else {
		    f = intexpr (argstck[arg + 2]);	/* 3rd arg */
		    if (ierr == MXNUM)	/* $J() arg number overflow	*/
			return;
		    if (f > (STRLEN - 2)) {
					/* $J() .precision too long	*/
			ierr = MXSTR;
			return;
		    }
		    numlit (a);
		    if (f < 0) {
			ierr = ARGER;
			return;
		    }
/* s j=$l(a),i=$f(a,".")-1 */
		    j = (a[0] == '-');
		    if (a[j] == '.') {	/* insert leading zero */
			i = j;
			while (a[i++] != EOL) ;
			while (i > j) {
			    a[i] = a[i - 1];
			    i--;
			}
			a[j] = '0';
		    }
		    i = (-1);
		    j = 0;
		    while (a[j] != EOL) {
			if (a[j] == '.')
			    i = j;
			j++;
		    }
		    if (i < 0) {
			a[i = j] = '.';
			a[j + 1] = EOL;
		    } else {
			j--;
		    }
		    if (j - i > f) {	/* rounding required */
			if ((l1 = f + i + 1) > STRLEN) {
			    ierr = MXSTR;
			    return;
			}
			if (a[l1] > '4') {
			    do {
				if (a[--l1] == '.')
				    l1--;
				if (l1 < (a[0] == '-')) {
				    for (l1 = f + i + 1; l1 > 0; l1--)
					a[l1] = a[l1 - 1];
				    a[a[0] == '-'] = '1';
				    i++;
				    break;
				}
				a[l1]++;
				if (a[l1] == ':')
				    a[l1] = '0';
			    } while (a[l1] == '0');
			}
			a[f + i + 1] = EOL;
			if (a[0] == '-' && a[1] == '0') {
			    l1 = 2;
			    while (a[l1] != EOL) {
				if (a[l1] >= '1' && a[l1] <= '9') {
				    l1 = 0;
				    break;
				}
				l1++;
			    }
			    if (l1) {
				i--;
				l1 = 0;
				while ((a[l1] = a[l1 + 1]) != EOL)
				    l1++;
			    }
			}
		    } else {
			if (f + i + 1 > STRLEN) {
			    ierr = MXSTR;
			    return;
			}
			while (j < f + i)
			    a[++j] = '0';
			a[++j] = EOL;
		    }
		    if (f == 0)
			a[i] = EOL;
		}			/* end of 3 arg-form */
		if (f < l) {
		    i = stlen (a) + 1;
		    if (++l <= i)
			goto nxt_operator;
		    while (i >= 0)
			a[l--] = a[i--];
		    while (l >= 0)
			a[l--] = SP;
		}
	    }
	    goto nxt_operator;
/* case 'd': *//* $DATA */
/* case 'g': *//* $GET */
/* case 'i': *//* $INCREMENT */
/* case 'n': *//* $NEXT */
/* case ZNEXT: *//* $ZNEXT */
/* case ZPREVIOUS: *//* $ZPREVIOUS */
	case 'o':			/* $ORDER */
	    if (f > 2) {
		ierr = FUNARG;
		return;
	    }
	    stcpy (varnam, argstck[arg]);
	    ordercnt = intexpr (argstck[arg + 1]);
	    ordercounter = 0;
	    if (varnam[0] != '^')
		symtab (order, varnam, a);
	    else if (varnam[1] != '$')
		global  (order, varnam, a);

	    else
		ssvn (order, varnam, a);
	    goto nxt_operator;

	case 'q':			/* $QUERY */
	    if (f > 2) {
		ierr = FUNARG;
		return;
	    }
	    stcpy (varnam, argstck[arg]);
	    ordercnt = intexpr (argstck[arg + 1]);
	    if (varnam[0] != '^')
		symtab (query, varnam, a);
	    else if (varnam[1] != '$')
		global  (query, varnam, a);

	    else
		ssvn (query, varnam, a);
	    goto nxt_operator;

	case 'N':			/* $NAME */
	    if (f > 2) {
		ierr = FUNARG;
		return;
	    }
	    f = intexpr (argstck[arg + 1]);
	    if (f < 0) {
		ierr = ARGER;
		return;
	    }
	    i = 0;
	    while (a[++i] != EOL)
		if (a[i] == DELIM && --f < 0)
		    break;
	    a[i] = EOL;
	    stcpy (varnam, a);
	    zname (a, varnam);

	    goto nxt_operator;

	case QLENGTH:			/* $QLENGTH */
	    if (f != 1) {
		ierr = FUNARG;
		return;
	    }
	    f = 0;
	    i = 0;
	    {
		int     ch,
		        quote;
		quote = 0;
		while ((ch = a[i++]) != EOL) {
		    if (ch == '"')
			quote = !quote;
		    if (quote)
			continue;
		    if (ch == '(' && f == 0)
			f = 1;
		    if (ch == ',')
			f++;
		}
	    }
	    intstr (a, f);
	    goto nxt_operator;

 case QSUBSCRIPT: /* $QSUBSCRIPT */
       if (f!=2) {
         ierr=FUNARG;
         return;
       }
       if ((f=intexpr(argstck[arg+1]))<-1) {
         ierr=ARGER;
         return;
       }
       { int ch,env,quote,count,startsub;
         if (f == -1) { /* get environment */
           quote=0; env=FALSE; count=0; startsub=0; i=0;
           while ((ch=a[i++])!=EOL) {
             if (ch=='"') quote= !quote;
             if (quote) continue;
             if (ch=='|') {
                if (env) {
                  a[i-1]=EOL;
                  stcpy(a,&a[startsub]);
                  break;
                }
                else {
                  startsub=i;
                  env=TRUE;
                }
             }
           }
          if (!env) *a=EOL;
         }
         else {
            quote=0; env=FALSE; count=0; startsub=0; i=0;
            while ((ch=a[i++])!=EOL) {
              if (ch=='"') quote= !quote;
              if (quote) continue;
              if (ch=='|' && count==0) {
                 if (env) {
                    if (*a=='^') a[--i]='^';
                    startsub=i;
                 }
              else env=TRUE;
            }
            if (ch=='(' || ch==',' || ch==')') {
               if (count==f) { a[i-1]=EOL; break; }
               count++; startsub=i;
	    }
         }
         if (startsub) stcpy(a,&a[startsub]);
            if (count<f) *a=EOL;
         }
         if (a[0]=='"') { /* un-quote */
            quote=1; 
            i=1; 
            f=0;
            while ((ch=a[i++])!=EOL) {
               if (ch=='"') quote=!quote;
               if (quote) a[f++]=ch;
            }
                  a[f]=EOL;
         }
       }

/* goto nxt_operator; */

	case 's':			/* $SELECT */
	    goto nxt_operator;

	case SVNstack:			/* $STACK() */
	    if (f > 2) {
		ierr = FUNARG;
		return;
	    }
	    if (f == 1) {
#ifdef DEBUG_NEWSTACK
                printf("Trying to get at nstx in expr.c!\r\n");
#endif
		intstr (a, nstx);
            }
	    goto nxt_operator;

	case FNUMBER:			/* $FNUMBER */

	    if (f < 2 || f > 3) {
		ierr = FUNARG;
		return;
	    } {
		short   l1;
		short   Pflag = FALSE,
		        Tflag = FALSE,
		        commaflag = FALSE,
		        plusflag = FALSE,
		        minusflag = FALSE,
		        EuroFlag = FALSE,
		        IsZero = FALSE;

		b = argstck[arg + 1];
		while ((i = *b++) != EOL) {	/* evaluate options */
		    switch (i) {
		    case 'P':
			Pflag = TRUE;
			continue;
		    case 'p':
			if (lowerflag)
			    Pflag = TRUE;
			continue;
		    case 'T':
			Tflag = TRUE;
			continue;
		    case 't':
			if (lowerflag)
			    Tflag = TRUE;
			continue;
		    case ',':
			commaflag = TRUE;
			continue;
		    case '.':
			EuroFlag = TRUE;
			continue;
		    case '+':
			plusflag = TRUE;
			continue;
		    case '-':
			minusflag = TRUE;
		    }
		}
		if (Pflag && (Tflag || plusflag || minusflag)) {
		    ierr = ARGER;
		    return;
		}
		if (f == 3)
		    j = intexpr (argstck[arg + 2]);	/* 3rd arg */
		if (ierr == MXNUM) {
		    if (j >= 0)
			j = 256;
		    ierr = OK;
		}
		numlit (a);
		IsZero = (a[0] == '0');
		if (f == 3) {
		    f = j;
		    if (f < 0) {
			ierr = ARGER;
			return;
		    }
		    if (f > STRLEN) {
			ierr = MXSTR;
			return;
		    }
/* s j=$l(a),i=$f(a,".")-1 */
		    j = (a[0] == '-');
		    if (a[j] == '.') {	/* insert leading zero */
			i = j;
			while (a[i++] != EOL) ;
			while (i > j) {
			    a[i] = a[i - 1];
			    i--;
			}
			a[j] = '0';
		    }
		    i = (-1);
		    j = 0;
		    while (a[j] != EOL) {
			if (a[j] == '.')
			    i = j;
			j++;
		    }
		    if (i < 0) {
			a[i = j] = '.';
			a[j + 1] = EOL;
		    } else {
			j--;
		    }
		    if (j - i > f) {	/* rounding required */
			l1 = f + i + 1;
			if (a[l1] > '4') {
			    do {
				if (a[--l1] == '.')
				    l1--;
				if (l1 < 0) {
				    for (l1 = f + i + 1; l1 > 0; l1--)
					a[l1] = a[l1 - 1];
				    a[0] = '1';
				    i++;
				    break;
				}
				a[l1]++;
				if (a[l1] == ':')
				    a[l1] = '0';
			    } while (a[l1] == '0');
			}
			a[f + i + 1] = EOL;
			if (a[0] == '-' && a[1] == '0') {
			    l1 = 2;
			    while (a[l1] != EOL) {
				if (a[l1] >= '1' && a[l1] <= '9') {
				    l1 = 0;
				    break;
				}
				l1++;
			    }
			    if (l1) {
				i--;
				l1 = 0;
				while ((a[l1] = a[l1 + 1]) != EOL)
				    l1++;
			    }
			}
		    } else {
			if (f + i > STRLEN) {
			    ierr = MXSTR;
			    return;
			}
			while (j < f + i)
			    a[++j] = '0';
			a[++j] = EOL;
		    }
		    if (f == 0)
			a[i] = EOL;
		}			/* end of 3 arg-form */
		if (commaflag) {
		    i = 0;
		    while ((f = a[i]) != '.' && f != EOL)
			i++;
		    if (a[0] == '-') {
			f = (i + 1) % 3;
			j = 1;
			i = 1;
			tmp[0] = '-';
		    } else {
			f = (i + 2) % 3;
			j = 0;
			i = 0;
		    }
		    while ((tmp[j++] = a[i]) != EOL) {
			if (j >= STRLEN) {
			    ierr = MXSTR;
			    return;
			}
			if (a[i++] == '.')
			    f = -1;	/* do not insert comma after point */
			if (f-- == 0 && a[i] != EOL && a[i] != '.') {
			    f = 2;
			    tmp[j++] = ',';
			}
		    }
		    stcpy (a, tmp);
		}
		if (EuroFlag && !standard) {	/* exchange point and comma */
		    i = 0;
		    while ((f = a[i]) != EOL) {
			if (f == '.')
			    a[i] = ',';
			if (f == ',')
			    a[i] = '.';
			i++;
		    }
		}
		if (Tflag) {
		    i = stcpy (tmp, a);
		    if (plusflag && tmp[0] != '-' && !IsZero) {
			tmp[i] = '+';
			tmp[++i] = EOL;
			stcpy (a, tmp);
		    } else if (tmp[0] == '-') {
			tmp[i] = minusflag ? SP : '-';
			tmp[++i] = EOL;
			stcpy (a, &tmp[1]);
		    } else {
			tmp[i] = SP;
			tmp[++i] = EOL;
			stcpy (a, tmp);
		    }
		    goto nxt_operator;
		}
		if (Pflag) {
		    i = stcpy (&tmp[1], a);
		    if (a[0] == '-') {
			a[0] = '(';
			a[i] = ')';
			a[++i] = EOL;
		    } else {
			tmp[0] = SP;
			tmp[++i] = SP;
			tmp[++i] = EOL;
			stcpy (a, tmp);
		    }
		    goto nxt_operator;
		}
		if (plusflag && a[0] != '-' && !IsZero) {
		    stcpy (tmp, a);
		    a[0] = '+';
		    stcpy (&a[1], tmp);
		}
		if (minusflag && a[0] == '-') {
		    stcpy (tmp, &a[1]);
		    stcpy (a, tmp);
		}
	    }
	    goto nxt_operator;

	case REVERSE:			/* $REVERSE */
	    if (f != 1) {
		ierr = FUNARG;
		return;
	    }
	    i = stlen (a) - 1;
	    j = i / 2;
	    i = i - j;
	    while (j >= 0) {
		f = a[j];
		a[j--] = a[i];
		a[i++] = f;
	    }

	    goto nxt_operator;

	case 't':			/* $TEXT */

	    {
		long    l1,
		        rouoldc;
		short   reload = FALSE;

		if (f > 3) {
		    ierr = FUNARG;
		    return;
		}
		i = 0;
		if (f > 1) {
		    stcpy (tmp, argstck[arg + 1]);
		    i = intexpr (tmp);
		}
		if (a[0] == EOL) {
		    if (i < 0) {
			ierr = ARGER;
			return;
		    }
/* $T(+0) returns routine name */
		    if (i == 0) {
			if (f != 3) {
			    i = 0;
			    while ((a[i] = rou_name[i]) != EOL) {
				if (rou_name[i] == '.')
				    break;
				i++;
			    }
			    a[i] = EOL;
			} else
			    stcpy (a, argstck[arg + 2]);
			goto nxt_operator;
		    }
		}
		if (f == 3) {
		    reload = TRUE;	/* load routine; */
		    f = mcmnd;
		    mcmnd = 'd';	/* make load use standard-path */
		    stcpy (tmp, argstck[arg + 2]);
		    rouoldc = roucur - rouptr;
		    zload (tmp);
		    mcmnd = f;
		    if (ierr > OK) {
			zload (rou_name);
			if (ierr == NOPGM || ierr == (NOPGM - CTRLB)) {
			    ierr -= NOPGM;
			    *a = EOL;
			    goto nxt_operator;
			}
			return;
		    }
		}
		j = 0;
		f = 1;
		if (a[0] != EOL) {	/* 1st arg == label */
		    for (;;)
		    {
			if (j >= (rouend - rouptr)) {
			    a[0] = EOL;
			    goto t_end;
			}
			l1 = j;
			f = 0;
			while (*(rouptr + (++l1)) == a[f++]) ;
			if (a[--f] == EOL && (*(rouptr + l1) == TAB ||
					      *(rouptr + l1) == SP ||
					      *(rouptr + l1) == '('))
			    break;
			j += (UNSIGN (*(rouptr + j)) + 2);	/* skip line */
		    }
		    f = 0;
		}
		if (i > 0) {
		    while (f < i) {
			if ((j = j + (UNSIGN (*(rouptr + j))) + 2) >= (rouend - rouptr)) {
			    a[0] = EOL;
			    goto t_end;
			}
			f++;
		    }
		}
		if (i < 0) {
		    j--;
		    while (f != i) {
			while (*(rouptr + (--j)) != EOL && j >= 0) ;
			if (--f != i && j < 1) {
			    a[0] = EOL;
			    goto t_end;
			}
		    }
		    j++;
		}
		f = (-1);
		j++;
		while ((a[++f] = (*(rouptr + (j++)))) != EOL) {
		    if (a[f] == TAB || a[f] == SP)
			break;
		}
		if (j >= (rouend - rouptr - 1)) {
		    a[0] = EOL;
		} else {
		    a[f] = SP;
		    while ((*(rouptr + j)) == TAB || (*(rouptr + j)) == SP) {
			j++;
			a[++f] = SP;
		    }
		    stcpy (&a[++f], rouptr + j);
		}
	      t_end:if (reload) {
		    zload (rou_name);
		    roucur = rouptr + rouoldc;
		}			/* reload routine; */
	    }
	    goto nxt_operator;

	case TRANSLATE:		/* $TRANSLATE */

	    if (f > 3 || f < 2) {
		ierr = FUNARG;
		return;
	    } {
		short   l1,
		        m;
		char   *c;

		b = argstck[arg + 1];
		c = argstck[arg + 2];
		if (f == 2)
		    l1 = 0;
		else
		    l1 = stlen (c);	/* $l of 3rd arg */
		m = 0;
		f = 0;
		while ((ch = a[f++]) != EOL) {
		    j = 0;
		    while (b[j] != EOL) {
			if (ch == b[j]) {
			    if (j < l1)
				ch = c[j];
			    else
				ch = EOL;
			    break;
			}
			j++;
		    }
		    if (ch != EOL)
			a[m++] = ch;
		}
		a[m] = EOL;
	    }
	    goto nxt_operator;

	case 'r':			/* $RANDOM */

	    if (f != 1) {
		ierr = FUNARG;
		return;
	    } {
		long    ilong;

		random = (ran_a * random + ran_b) % ran_c;
		if ((i = intexpr (a)) < 1) {
		    ierr = ARGER;
		    return;
		}
		ilong = (random * i) / ran_c;
		if (ilong < 0)
		    ilong += i;
		lintstr (a, ilong);
	    }
	    goto nxt_operator;

/* $VIEW */
	case 'v':
	    view_fun (f, a);
	    if (ierr > 0)
		return;
	    goto nxt_operator;

/* $ZBOOLEAN */
	case 'B':
	    if (f != 3) {
		ierr = FUNARG;
		return;
	    }
	    i = 0;
	    ch = intexpr (argstck[arg + 2]) % 16;
	    b = argstck[arg + 1];
	    if (*b == EOL) {
		*b = 0;
		b[1] = 0;
	    }
	    f = 0;
	    switch (ch) {
/* 1: A AND B */
	    case 1:
		while (a[i] != EOL) {
		    a[i] &= b[f];
		    i++;
		    if (b[++f] == EOL)
			f = 0;
		}
		break;
/* 7: A OR B */
	    case 7:
		while (a[i] != EOL) {
		    a[i] |= b[f];
		    i++;
		    if (b[++f] == EOL)
			f = 0;
		}
		break;
/* 6: A XOR B */
	    case 6:
		while (a[i] != EOL) {
		    a[i] = (a[i] ^ b[f]) & (eightbit ? 0377 : 0177);
		    i++;
		    if (b[++f] == EOL)
			f = 0;
		}
		break;
/* 14: A NAND B */
	    case 14:
		while (a[i] != EOL) {
		    a[i] = ~(a[i] & b[f]) & (eightbit ? 0377 : 0177);
		    i++;
		    if (b[++f] == EOL)
			f = 0;
		}
		break;
/* 8: A NOR B */
	    case 8:
		while (a[i] != EOL) {
		    a[i] = ~(a[i] | b[f]) & (eightbit ? 0377 : 0177);
		    i++;
		    if (b[++f] == EOL)
			f = 0;
		}
		break;
/* 9: A EQUALS B */
	    case 9:
		while (a[i] != EOL) {
		    a[i] = ~(a[i] ^ b[f]) & (eightbit ? 0377 : 0177);
		    i++;
		    if (b[++f] == EOL)
			f = 0;
		}
		break;
/* 2: A AND NOT B */
	    case 2:
		while (a[i] != EOL) {
		    a[i] &= ~b[f];
		    i++;
		    if (b[++f] == EOL)
			f = 0;
		}
		break;
/* 11: A OR NOT B */
	    case 11:
		while (a[i] != EOL) {
		    a[i] = (a[i] | ~b[f]) & (eightbit ? 0377 : 0177);
		    i++;
		    if (b[++f] == EOL)
			f = 0;
		}
		break;
/* 13: NOT A OR B */
	    case 13:
		while (a[i] != EOL) {
		    a[i] = (~a[i] | b[f]) & (eightbit ? 0377 : 0177);
		    i++;
		    if (b[++f] == EOL)
			f = 0;
		}
		break;
/* 4: NOT A AND B */
	    case 4:
		while (a[i] != EOL) {
		    a[i] = ~a[i] & b[f];
		    i++;
		    if (b[++f] == EOL)
			f = 0;
		}
		break;
/* 5: B */
	    case 5:
		while (a[i] != EOL) {
		    a[i++] = b[f];
		    if (b[++f] == EOL)
			f = 0;
		}
		break;
/* 10: NOT B */
	    case 10:
		while (a[i] != EOL) {
		    a[i++] = ~b[f] & 0177;
		    if (b[++f] == EOL)
			f = 0;
		}
		break;
/* 12: NOT A */
	    case 12:
		while (a[i] != EOL) {
		    a[i] = ~a[i] & 0177;
		    i++;
		    if (b[++f] == EOL)
			f = 0;
		}
		break;
/* 0: always FALSE */
	    case 0:
		while (a[i] != EOL)
		    a[i++] = 0;
		break;
/* 15: always TRUE */
	    case 15:
		ch = (char) 0177;
		while (a[i] != EOL)
		    a[i++] = ch;
/* 3: A */
	    }
	    goto nxt_operator;

/* ZCRC "cyclic redundancy check" check sums */
	case ZCRC:
	    if (f == 1)
		f = 0;			/* missing 2nd arg defaults to "0" */
	    else {
		if (f != 2) {
		    ierr = FUNARG;
		    return;
		}
		if ((f = intexpr (argstck[arg + 1])) != 0 &&
		    f != 1) {
		    ierr = ARGER;
		    return;
		}
	    }
	    i = 0;
	    if (f == 0) {		/* XORing */
		f = 0;
		while (a[i] != EOL)
		    f ^= a[i++];
		f = f & 0377;
	    } else {			/* ASCII sum */
		f = 0;
		while (a[i] != EOL)
		    f += a[i++];
	    }
	    intstr (a, f);
	    goto nxt_operator;

/* $ZFUNCTIONKEY */
	case 'F':
	    if (f != 1) {
		ierr = FUNARG;
		return;
	    }
	    if ((i = intexpr (a)) < 1 || i > 44) {
		ierr = FUNARG;
		return;
	    }
	    stcpy (a, zfunkey[i - 1]);
	    goto nxt_operator;

	case 'P':			/* $ZPIECE */

/* Similar to $PIECE                                    */
/* The difference is, that stuff within quotes is not   */
/* counted as delimiter. nor is stuff within brackets   */

	    {
		short   l,
		        l1,
		        m,
		        n;
		short   quo = 0;	/* quotes */
		short   bra = 0;	/* brackets */
		char    ch0;

		b = argstck[arg + 1];
		l1 = b - a - 1;		/* length of 1st argument */

		switch (f) {
		case 2:
		    f = 1;
		    l = 1;
		    break;
		case 3:
		    if ((f = intexpr (argstck[arg + 2])) <= 0) {
			a[0] = EOL;
			goto nxt_operator;
		    }
		    if (ierr == MXNUM) {
			if (f >= 0)
			    f = 256;
			ierr = OK;
		    }
		    l = f;
		    break;
		case 4:
		    l = intexpr (argstck[arg + 3]);
		    if (ierr == MXNUM) {
			if (l >= 0)
			    l = 256;
			ierr = OK;
		    }
		    if ((f = intexpr (argstck[arg + 2])) <= 0)
			f = 1;
		    if (ierr == MXNUM) {
			if (f >= 0)
			    f = 256;
			ierr = OK;
		    }
		    if (f > l) {
			a[0] = EOL;
			goto nxt_operator;
		    }
		    break;
		default:
		    ierr = FUNARG;
		    return;
		}

		i = 0;
		m = 0;
		ch = 0;
		while (b[ch] != EOL)
		    ch++;		/* $l of 2nd arg */

		if (ch == 1) {
		    ch = b[0];
		    j = 1;
		    if (f > 1) {
			while (i < l1) {	/* scan 1st string ... */
			    ch0 = a[i++];
			    if (ch != '"') {
				if (ch0 == '"') {
				    toggle (quo);
				    continue;
				}
				if (quo)
				    continue;
			    }
			    if (ch0 == '(')
				bra++;
			    if (ch0 == ')')
				bra--;
			    if (ch0 != ch)
				continue;
			    if (bra > 1)
				continue;
			    if ((ch0 != '(') && bra)
				continue;
			    if (++j == f) {
				m = i;
				goto zp10;
			    }
			}
/* if(j<f) */ a[0] = EOL;
			goto nxt_operator;
		    }
		  zp10:
		    for (; i < l1; i++) {
			ch0 = a[i];
			if (ch != '"') {
			    if (ch0 == '"') {
				toggle (quo);
				continue;
			    }
			    if (quo)
				continue;
			}
			if (ch0 == '(')
			    bra++;
			if (ch0 == ')')
			    bra--;
			if (ch0 != ch)
			    continue;
			if (bra > 1)
			    continue;
			if ((ch0 != '(') && bra)
			    continue;

			if (j == l) {
			    a[i] = EOL;
			    break;
			}
			j++;
		    }
		    if (m > 0)
			stcpy (a, &a[m]);
		    goto nxt_operator;
		}
		if (ch == 0) {
		    a[0] = EOL;
		    goto nxt_operator;
		}			/* 2nd arg is empty */
/* else (ch>1) $Length of Delimiter>1 */
		n = 1;
		if (f > 1) {
		    while (i < l1) {	/* scan 1st string ... */
			j = 0;
			if ((ch0 = a[i]) == '"') {
			    toggle (quo);
			    i++;
			    continue;
			}
			if (quo) {
			    i++;
			    continue;
			}
			if (ch0 == '(') {
			    bra++;
			    i++;
			    continue;
			}
			if (ch0 == ')') {
			    bra--;
			    i++;
			    continue;
			}
			if (bra) {
			    i++;
			    continue;
			}
		      zp20:if (a[i + j] != b[j]) {
			    i++;
			    continue;
			}		/* ... for occurence of 2nd */
			if (++j < ch)
			    goto zp20;
			i += ch;	/* skip delimiter */
			if (++n == f) {
			    m = i;
			    goto zp30;
			}
		    }
/* if(n<f) */ a[0] = EOL;
		    goto nxt_operator;
		}
	      zp30:while (i < l1) {
		    j = 0;
		    if ((ch0 = a[i]) == '"') {
			toggle (quo);
			i++;
			continue;
		    }
		    if (quo) {
			i++;
			continue;
		    }
		    if (ch0 == '(') {
			bra++;
			i++;
			continue;
		    }
		    if (ch0 == ')') {
			bra--;
			i++;
			continue;
		    }
		    if (bra) {
			i++;
			continue;
		    }
		  zp40:if (a[i + j] != b[j]) {
			i++;
			continue;
		    }
		    if (++j < ch)
			goto zp40;
		    if (n == l) {
			a[i] = EOL;
			break;
		    }			/* last $zpiece: done! */
		    i += ch;
		    n++;
		}
		if (m > 0)
		    stcpy (a, &a[m]);
		goto nxt_operator;
	    }

	case 'L':			/* $ZLENGTH */

/* Similar to $LENGTH with two arguments                */
/* The difference is, that stuff within quotes is not   */
/* counted as delimiter. nor is stuff within brackets   */

	    if (f != 2) {
		ierr = FUNARG;
		return;
	    }
	    i = 0;
	    j = 0;
	    b = argstck[arg + 1];
	    if ((f = stlen (b))) {
		int     quo,
		        bra,
		        ch0;

		quo = 0;
		bra = 0;
		if (f == 1) {		/* length of delimiter =1 char */
		    ch = b[0];
		    j = 0;
		    for (;;)
		    {
			ch0 = a[i++];
			if (ch0 == EOL)
			    break;
			if (ch != '"') {
			    if (ch0 == '"') {
				toggle (quo);
				continue;
			    }
			    if (quo)
				continue;
			}
			if (ch0 == '(')
			    bra++;
			if (ch0 == ')')
			    bra--;
			if (ch0 != ch)
			    continue;
			if (bra > 1)
			    continue;
			if ((ch0 != '(') && bra)
			    continue;
			j++;
		    }
		    j++;
		} else {
		    int     n;

		    j = 1;
		    for (;;)
		    {
			n = 0;
			if ((ch0 = a[i]) == '"') {
			    toggle (quo);
			    i++;
			    continue;
			}
			if (ch0 == EOL)
			    break;
			if (quo) {
			    i++;
			    continue;
			}
			if (ch0 == '(') {
			    bra++;
			    i++;
			    continue;
			}
			if (ch0 == ')') {
			    bra--;
			    i++;
			    continue;
			}
			if (bra) {
			    i++;
			    continue;
			}
		      zl10:if (a[i + n] != b[n]) {
			    i++;
			    continue;
			}
			if (++n < f)
			    goto zl10;
			i += f;		/* skip delimiter */
			j++;
		    }
		}
	    }
	    intstr (a, j);
	    goto nxt_operator;

	case ZLSD:			/* $ZLSD levenshtein function */
	    if (f != 2) {
		ierr = FUNARG;
		return;
	    }
	    f = levenshtein (a, argstck[arg + 1]);
	    intstr (a, f);
	    goto nxt_operator;

/* $ZKEY */
/* transform a string to be used as a key in an array so   */
/* the result string will collate in the desired way       */
/* according to the production rule specified by VIEW 93   */
	case 'K':
	    if (f == 2)
		zkey (a, intexpr (argstck[arg + 1]));
	    else if (f == 1)
		zkey (a, v93);
	    else
		ierr = FUNARG;
	    if (ierr > OK)
		return;
	    goto nxt_operator;

/* $ZREPLACE */
/* Replace in first argument non overlapping occurences    */
/* of the second argument by the third argument.           */
/* if the third argument is missing, assume it to be empty */
	case 'R':
	    if (f == 3)
		zreplace (a, argstck[arg + 1], argstck[arg + 2]);
	    else if (f == 2)
		zreplace (a, argstck[arg + 1], "\201");
	    else
		ierr = FUNARG;
	    if (ierr > OK)
		return;
	    goto nxt_operator;

/* $ZSYNTAX */

	case 'S':
	    if (f != 1) {
		ierr = FUNARG;
		return;
	    }
	    zsyntax (a);
	    if (ierr > OK)
		return;
	    goto nxt_operator;

/* $ZDATE() */

	case 'D':
	    if (f > 2) {
		ierr = FUNARG;
		return;
	    }
	    j = (f == 2 ? intexpr (argstck[arg + 1]) : datetype);
	    if (j < 0 || j >= NO_DATETYPE) {
		ierr = ARGER;
		return;
	    }
	    zdate (a, (long) intexpr (a), j);
	    if (ierr > 0)
		return;
	    goto nxt_operator;

/* $ZHOROLOG() */
/* convert string date to $H format */
	case 'H':
	    if (f > 2) {
		ierr = FUNARG;
		return;
	    }
	    j = (f == 2 ? intexpr (argstck[arg + 1]) : datetype);
	    if (j < 0 || j >= NO_DATETYPE) {
		ierr = ARGER;
		return;
	    } {
		long    d,
		        dn,
		        m,
		        y,
		        lp,
		        ilong,
		        k;
		char    c;
		long    dn1,
		        m1,
		        lp1;

		i = 0;
		if (*a == EOL) {
		    ierr = ARGER;
		    return;
		}
		if (dat4flag[j] == 2) {	/* Y-M-D format */
/* problem: if there is no 1st.delimiter and if MM is numeric, then
 * it is not trivial to find the boundary between YY and MM
 * in that case MM must be two chars, otherwise the algorithm
 * won't work.
 */
		    m = 0;		/* here used as flag... */
		    if (dat1char[j][0] == EOL) {
			for (m = 0; m < 12; m++) {
			    if ((c = month[j][m][0]) >= '0' && c <= '9')
				break;
			}
			if (m < 12) {	/* the problematic case ! */
			    if (dat2char[j][0] == EOL)
				m = stlen (a) - 4;
			    else
				m = find (a, dat2char[j]) - 3;
			    if (m <= 0) {
				ierr = ARGER;
				return;
			    }
			    y = 0;
			    i = 0;
			    while ((c = a[i]) >= '0' && c <= '9') {
				c -= '0';
				dn = y * 10 + c;
				ilong = dn * 365;
				if (((ilong / 365 - c) / 10) != y) {
				    ierr = ARGER;
				    return;
				}
				y = dn;
				i++;
				if (i >= m)
				    break;
			    }
			} else
			    m = 0;
		    }
		    if (m == 0) {
			y = 0;
			i = 0;
			while ((c = a[i]) >= '0' && c <= '9') {
			    c -= '0';
			    dn = y * 10 + c;
			    ilong = dn * 365;
			    if (((ilong / 365 - c) / 10) != y) {
				ierr = ARGER;
				return;
			    }
			    y = dn;
			    i++;
			}
		    }
/* 2 digits is current century */
		    if (i == 2 && dat5flag[j])
			y += ((((time (0L) + tzoffset) / 86400) > 10956) ? 2000 : 1900);

		    if (dat1char[j][0] != EOL) {
			k = 0;
			while (dat1char[j][k] == a[i]) {
			    if (a[i] == EOL) {
				ierr = ARGER;
				return;
			    }
			    k++;
			    i++;
			}
			if (dat1char[j][k] != EOL) {
			    ierr = ARGER;
			    return;
			}
		    }
		    c = dat2char[j][0];
		    for (m = 0; m < 12; m++) {
			if (find (&a[i], month[j][m]) != 1) {
			    if (m == 11 && a[i] == '0') {
				i++;
				m = (-1);
			    }		/* leading zero */
			    if (m == 11 && month[j][8][0] == '0' && a[i] >= '1' && a[i] <= '9') {
				m = a[i++] - '1';
				break;
			    }		/* leading zero */
			    continue;
			}
			k = stlen (month[j][m]);
			if (c == EOL) {
			    i += k;
			    break;
			}
			if (c == a[i + k]) {
			    i += k;
			    break;
			}
		    }
		    if (m >= 12) {
			ierr = ARGER;
			return;
		    }
		    m++;
		    k = 0;
		    while (a[i] == dat2char[j][k]) {
			if (a[i] == EOL) {
			    ierr = ARGER;
			    return;
			}
			i++;
			k++;
		    }
		    if (dat2char[j][k] != EOL) {
			ierr = ARGER;
			return;
		    }
		    c = a[i++];
		    d = 0;
		    if (c >= '0' && c <= '9')
			d = c - '0';
		    else if (c != dat3char[j]) {
			ierr = ARGER;
			return;
		    }
		    c = a[i++];
		    if (c >= '0' && c <= '9') {
			d = d * 10 + c - '0';
			c = a[i];
		    }
		    if (c != EOL) {
			ierr = ARGER;
			return;
		    }
		} else {
		    if (dat4flag[j]) {	/* M-D-Y format */
			c = dat1char[j][0];
			i = 0;
			for (m = 0; m < 12; m++) {
			    if (find (&a[i], month[j][m]) != 1) {
				if (m == 11 && a[i] == '0') {
				    i++;
				    m = (-1);
				}	/* leading zero */
				continue;
			    }
			    i += stlen (month[j][m]);
			    k = 0;
			    while (a[i + k] == dat1char[j][k]) {
				if (a[y + k] == EOL) {
				    ierr = ARGER;
				    return;
				}
				k++;
			    }
			    if (dat1char[j][k] == EOL) {
				i += k;
				break;
			    }
			}
			if (m >= 12) {
			    ierr = ARGER;
			    return;
			}
			m++;
			c = a[i++];
			d = 0;
			if (c >= '0' && c <= '9')
			    d = c - '0';
			else if (c != dat3char[j]) {
			    ierr = ARGER;
			    return;
			}
			c = a[i];
			if (c >= '0' && c <= '9') {
			    d = d * 10 + c - '0';
			    c = a[++i];
			}
		    } else if (dat4flag[j] == 0) {	/* D-M-Y format */
			c = a[i++];
			d = 0;
			if (c >= '0' && c <= '9')
			    d = c - '0';
			else if (c != dat3char[j]) {
			    ierr = ARGER;
			    return;
			}
			c = a[i];
			if (c >= '0' && c <= '9') {
			    d = d * 10 + c - '0';
			    c = a[++i];
			}
			if (dat1char[j][0] != EOL) {
			    k = 0;
			    while (dat1char[j][k] == a[i]) {
				if (a[i] == EOL) {
				    ierr = ARGER;
				    return;
				}
				k++;
				i++;
			    }
			    if (dat1char[j][k] != EOL) {
				ierr = ARGER;
				return;
			    }
			}
			c = dat2char[j][0];
			for (m = 0; m < 12; m++) {
			    if (find (&a[i], month[j][m]) != 1) {
				if (m == 11 && a[i] == '0') {
				    i++;
				    m = (-1);
				}	/* leading zero */
				continue;
			    }
			    k = stlen (month[j][m]);
			    if (c == EOL) {
				i += k;
				break;
			    }
			    if (c == a[i + k]) {
				i += k;
				break;
			    }
			}
			if (m >= 12) {
			    ierr = ARGER;
			    return;
			}
			m++;
		    }
		    k = 0;
		    while (a[i] == dat2char[j][k]) {
			if (a[i] == EOL) {
			    ierr = ARGER;
			    return;
			}
			i++;
			k++;
		    }
		    if (dat2char[j][k] != EOL) {
			ierr = ARGER;
			return;
		    }
		    y = 0;
		    k = 0;
		    while ((c = a[i++]) >= '0' && c <= '9') {
			c -= '0';
			dn = y * 10 + c;
			ilong = dn * 365;
			if (((ilong / 365 - c) / 10) != y) {
			    ierr = ARGER;
			    return;
			}
			y = dn;
			k++;
		    }
		    if (c != EOL) {
			ierr = ARGER;
			return;
		    }
/* 2 digits is current century */
		    if (k == 2 && dat5flag[j])
			y += ((((time (0L) + tzoffset) / 86400) > 10956) ? 2000 : 1900);
		}
		if (m < 1 || m > 12 || d < 1 || y <= 0) {
		    ierr = ARGER;
		    return;
		}
		dn = d + y * 365 + (y - 1) / 4 - (y - 1) / 100 + (y - 1) / 400;
		lp = ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0);
		if (y > 9) {
		    dn1 = d + y * 365 + (y - 1) / 4 - 2;
		    lp1 = (y % 4 == 0);
		} else {
		    dn1 = d + y * 365;
		    lp1 = FALSE;
		}
		m1 = m;

/* first check julian calendar */
		i = 31;
		if (--m1 == 0)
		    goto zd2;
		dn1 += i;
		i = 28 + lp1;
		if (--m1 == 0)
		    goto zd2;
		dn1 += i;
		i = 31;
		if (--m1 == 0)
		    goto zd2;
		dn1 += i;
		i = 30;
		if (--m1 == 0)
		    goto zd2;
		dn1 += i;
		i = 31;
		if (--m1 == 0)
		    goto zd2;
		dn1 += i;
		i = 30;
		if (--m1 == 0)
		    goto zd2;
		dn1 += i;
		i = 31;
		if (--m1 == 0)
		    goto zd2;
		dn1 += i;
		if (--m1 == 0)
		    goto zd2;
		dn1 += i;
		i = 30;
		if (--m1 == 0)
		    goto zd2;
		dn1 += i;
		i = 31;
		if (--m1 == 0)
		    goto zd2;
		dn1 += i;
		i = 30;
		if (--m1 == 0)
		    goto zd2;
		dn1 += i;
		i = 31;
		if (--m1 == 0)
		    goto zd2;
		dn1 += i;
	      zd2:if (dn1 < datGRbeg[j]) {	/* no doubt: it is before the reform */
		    if (d > i || dn1 < 0) {
			ierr = ARGER;
			return;
		    }
		    dn = dn1;
		    goto zd4;
		}
/* then check gregorian calendar */
		i = 31;
		if (--m == 0)
		    goto zd3;
		dn += i;
		i = 28 + lp;
		if (--m == 0)
		    goto zd3;
		dn += i;
		i = 31;
		if (--m == 0)
		    goto zd3;
		dn += i;
		i = 30;
		if (--m == 0)
		    goto zd3;
		dn += i;
		i = 31;
		if (--m == 0)
		    goto zd3;
		dn += i;
		i = 30;
		if (--m == 0)
		    goto zd3;
		dn += i;
		i = 31;
		if (--m == 0)
		    goto zd3;
		dn += i;
		if (--m == 0)
		    goto zd3;
		dn += i;
		i = 30;
		if (--m == 0)
		    goto zd3;
		dn += i;
		i = 31;
		if (--m == 0)
		    goto zd3;
		dn += i;
		i = 30;
		if (--m == 0)
		    goto zd3;
		dn += i;
		i = 31;
		if (--m == 0)
		    goto zd3;
		dn += i;
	      zd3:if (d > i || dn < datGRbeg[j]) {
		    ierr = ARGER;
		    return;
		}
	      zd4:lintstr (a, dn -= 672411L);
	    }
	    goto nxt_operator;

/* ZTIME */
	case 'T':
	    if (f > 2) {
		ierr = FUNARG;
		return;
	    }
	    j = (f == 2 ? intexpr (argstck[arg + 1]) : timetype);
	    if (j < 0 || j >= NO_TIMETYPE) {
		ierr = ARGER;
		return;
	    }
	    ztime (a, (long) intexpr (a), j);
	    if (ierr > 0)
		return;
	    goto nxt_operator;

/* ZHT */
	case ZHTIME:
	    if (f > 2) {
		ierr = FUNARG;
		return;
	    }
	    j = (f == 2 ? intexpr (argstck[arg + 1]) : timetype);
	    if (j < 0 || j >= NO_TIMETYPE) {
		ierr = ARGER;
		return;
	    } {
		long    hrs,
		        min,
		        sec;
		char    c;

		i = 0;
		if (*a == EOL) {
		    ierr = ARGER;
		    return;
		}
		c = a[i++];
		hrs = 0;
		if (c >= '0' && c <= '9')
		    hrs = c - '0';
		else if (c != tim3char[j]) {
		    ierr = ARGER;
		    return;
		}
		c = a[i];
		if (c >= '0' && c <= '9') {
		    hrs = hrs * 10 + c - '0';
		    c = a[++i];
		}
		if (tim1char[j] != EOL) {
		    if (c != tim1char[j]) {
			ierr = ARGER;
			return;
		    }
		    i++;
		}
		if ((c = a[i++]) < '0' || c > '5') {
		    ierr = ARGER;
		    return;
		}
		min = c - '0';
		if ((c = a[i++]) < '0' || c > '9') {
		    ierr = ARGER;
		    return;
		}
		min = min * 10 + c - '0';
		c = a[i];
		if (tim5flag[j])
		    sec = 0;
		else if (c == EOL || (c != tim2char[j]))
		    sec = 0;
		else {
		    if (tim2char[j] != EOL) {
			if (c != tim2char[j]) {
			    ierr = ARGER;
			    return;
			}
			i++;
		    }
		    if ((c = a[i++]) < '0' || c > '5') {
			ierr = ARGER;
			return;
		    }
		    sec = c - '0';
		    if ((c = a[i++]) < '0' || c > '9') {
			ierr = ARGER;
			return;
		    }
		    sec = sec * 10 + c - '0';
		}
		if (tim4flag[j]) {
		    if (a[i++] != SP || ((c = a[i++]) != 'A' && c != 'P') || (a[i++] != 'M')) {
			ierr = ARGER;
			return;
		    }
		    if (hrs == 12)
			hrs -= 12;
		    if (c == 'P')
			hrs += 12;
		}
		if (a[i] != EOL || hrs >= 24) {
		    ierr = ARGER;
		    return;
		}
		lintstr (a, hrs * 3600 + min * 60 + sec);
	    }
	    if (ierr > 0)
		return;
	    goto nxt_operator;

	case GETX:			/* dummy function for implicit $GET */
/* un-stack $ZREFERENCE and $ZLOCAL */
	    stcpy (zref, refsav[--refsx]);
	    stcpy (zloc, refsav[refsx] + 256);
	    free (refsav[refsx]);
	case GET:			/* dummy function for $GET with two args */
	    goto nxt_operator;

	case 'E':			/* ZEDIT */
	    if (f > 4) {
		ierr = FUNARG;
		return;
	    } {
		int     k,
		        l,
		        rev,
		        esc;

		if (f == 1) {
		    rev = TRUE;
		    goto reverse;
		}
		j = (f == 4 ? intexpr (argstck[arg + 3]) : 1);	/* type of action */
		if ((rev = j < 0))
		    j = (-j);
		if ((esc = j / 10) == 1 || esc == 2)
		    j = j % 10;
		if (j < 1 || j > 3) {
		    ierr = ARGER;
		    return;
		}
		f = (f >= 3 ? intexpr (argstck[arg + 2]) : 0);	/* target length */
		if (f > 255)
		    ierr = ARGER;
		if (ierr > OK)
		    return;
		if (esc == 1) {		/* remove ESC-Sequences */
		    stcpy (tmp, a);
		    i = 0;
		    k = 0;
		    l = 1;
		    esc = 0;
		    while ((a[k] = tmp[i++]) != EOL) {
			if (l) {
			    if (a[k] != ESC) {
				k++;
				continue;
			    }
			    if ((a[k] = tmp[i++]) != '[')
				continue;
			    l = 0;
			    continue;
			}
			if (a[k] >= '@')
			    l = 1;
		    }
		}
/* anything to be done ??? */
		if (argstck[arg + 1][0] == EOL)
		    goto reverse;
		stcpy (tmp, argstck[arg + 1]);
		if (j != 1) {		/* remove leading characters */
		    i = 0;
		    k = 0;
		    while (a[i] != EOL) {
			if (a[i] == tmp[k]) {
			    i++;
			    k = 0;
			    continue;
			}
			if (tmp[k++] == EOL)
			    break;
		    }
		    if (i)
			stcpy (a, &a[i]);
		}
		if (j != 3) {		/* remove trailing characters */
		    i = stlen (a) - 1;
		    k = 0;
		    while (i >= 0) {
			if (a[i] == tmp[k]) {
			    i--;
			    k = 0;
			    continue;
			}
			if (tmp[k++] == EOL)
			    break;
		    }
		    a[i + 1] = EOL;
		}
		i = stlen (a);
		if ((f -= i) > 0) {	/* characters to append */
		    if (esc == 2) {	/* ignore ESC-Sequences */
			k = 0;
			l = 1;
			while (a[k] != EOL) {
			    if (l) {
				if (a[k++] == ESC) {
				    f += 2;
				    if (a[k++] == '[')
					l = 0;
				}
			    } else {
				f++;
				if (a[k++] >= '@')
				    l = 1;
			    }
			}
		    }
		    k = 0;
		    if (j == 1) {
			k = f;
			f = 0;
		    }
		    if (j == 2) {
			k = f - f / 2;
			f -= k;
		    }
		    l = stlen (tmp);
		    if (k) {		/* append on right side */
			a[k += i] = EOL;
			j = l;
			while (--k >= i) {
			    a[k] = tmp[--j];
			    if (j <= 0)
				j = l;
			}
		    }
		    if (f) {		/* append on left side */
			i = 0;
			while (l < f)
			    tmp[l++] = tmp[i++];
			stcpy (&tmp[l], a);
			stcpy (a, tmp);
		    }
		}
	      reverse:;
		if (rev) {
		    i = stlen (a) - 1;
		    j = 0;
		    f = i / 2;
		    while (j <= f) {
			k = a[j];
			a[j++] = a[i];
			a[i--] = k;
		    }
		}
	    }
	    goto nxt_operator;

	default:
	    ierr = ILLFUN;
	    return;
	}
/* end of function evaluation section */
/******************************************************************************/

      nxt_operator:

	if (spx > 0 &&
	    (f = op_stck[spx]) != ARRAY &&
	    f != '(') {
	    goto nxt_expr;
	}
/* push answer */

	op_stck[++spx] = OPERAND;
	codptr++;
	goto nextchr;


    case '$':				/* scan function name convert upper to lower */

	if (op_stck[spx] == OPERAND)
	    goto m_operator;

	if ((f = *++codptr) >= 'A' && f <= 'Z')
	    f += 32;
	if (f == 'z' && standard) {
	    ierr = NOSTAND;
	    return;
	}
	if (f == '$') {			/* extrinsic function/extrinsic variable */
	    zexflag = FALSE;
	  extra_fun:;
	    {
		short   savmcmnd,
		        savsetp;	/* stuff to be saved */
		char    savarnam[256];
		char   *savdofr;
		long    savlen;
		short   savtest;
		short   savop;
		char   *savargs;
		int     savarg;
		char   *savastck;
		char   *savpart;
		char   *b;
		char   *namold;
		long    rouoldc;
		char    label[20],
		        routine[20];
		short   errex;		/* FLAG: error exit */

		savmcmnd = mcmnd;
		savsetp = setpiece;
		savop = setop;
		savtest = test;
		stcpy (savarnam, varnam);
		savdofr = dofram0;
		errex = FALSE;
		if ((argstck[++arg] = a) >= s) {
		    char   *bak;

		    bak = partition;
		    if (getpmore == 0) {
			ierr = STKOV;
			return;
		    }
		    a = a - bak + partition;
		    b = b - bak + partition;
		}
		savlen = a - argptr;
		savpart = partition;
		if (spx > 0) {
		    if ((savargs = calloc ((unsigned) (savlen + 256L), 1)) == NULL) {
			ierr = STKOV;
			return;
		    }			/* could not allocate stuff...     */
		    stcpy0 (savargs, argptr, savlen + 256L);
		    argptr = partition;
		}
		savarg = arg;
		if ((savastck = calloc ((unsigned) (arg + 1), sizeof (char *))) == NULL) {
		    ierr = STKOV;
		    return;
		}			/* could not allocate stuff...     */
		stcpy0 (savastck, (char *) argstck, (long) ((arg + 1) * sizeof (char *)));

		b = label;		/* parse label */
		if ((ch = *++codptr) == '%') {
		    *b++ = ch;
		    codptr++;
		}
		while (((ch = *codptr) >= 'A' && ch <= 'Z') ||
		       (ch >= 'a' && ch <= 'z') ||
		       (ch >= '0' && ch <= '9')) {
		    *b++ = ch;
		    codptr++;
		}
		*b = EOL;
		b = routine;
		if (ch == '^') {	/* parse routine name */
		    if (((ch = *++codptr) >= 'A' && ch <= 'Z') ||
			(ch >= 'a' && ch <= 'z') ||
			ch == '%')
			*b++ = ch;
		    while (((ch = *++codptr) >= 'A' && ch <= 'Z') ||
			   (ch >= 'a' && ch <= 'z') ||
			   (ch >= '0' && ch <= '9'))
			*b++ = ch;

		    if (routine[0] == EOL) {
			ierr = ILLFUN;
			errex = TRUE;
			goto errexfun;
		    }
		}
		*b = EOL;
/* something must be specified */
		if (label[0] == EOL && routine[0] == EOL) {
		    ierr = ILLFUN;
		    errex = TRUE;
		    goto errexfun;
		}
		if (*codptr == '(' && *(codptr + 1) != ')') {
		    dofram0 = dofrmptr;
		    i = 0;
		    codptr++;
		    for (;;)
		    {
			setpiece = TRUE;	/* to avoid error on closing bracket */
			if (*codptr == '.' && (*(codptr + 1) < '0' || *(codptr + 1) > '9')) {
			    codptr++;
			    expr (NAME);
			    codptr++;
			    *dofrmptr++ = DELIM;	/* to indicate call by name */
			    dofrmptr += stcpy (dofrmptr, varnam) + 1;
			} else {
			    expr (STRING);
			    dofrmptr += stcpy (dofrmptr, argptr) + 1;
			}
			setpiece = FALSE;
			i++;
			if (ierr > OK) {
			    dofrmptr = dofram0;
			    errex = TRUE;
			    goto errexfun;
			}
			ch = *codptr++;
			if (ch == ',')
			    continue;
			if (ch != ')') {
			    ierr = COMMAER;
			    dofrmptr = dofram0;
			    errex = TRUE;
			    goto errexfun;
			}
			ch = *codptr;
			break;
		    }
		} else {
		    dofram0 = 0;
		    if (*codptr == '(')
			codptr += 2;
		}

		rouoldc = roucur - rouptr;
		namold = 0;

		if (routine[0] != EOL) {	/* load routine */
		    dosave[0] = EOL;
		    loadsw = TRUE;
		    while ((*(namptr++)) != EOL) ;
		    namold = namptr;
		    stcpy (namptr, rou_name);
		    zload (routine);
		    if (ierr > OK) {
			errex = TRUE;
			goto errexfun;
		    }
		} {
		    char   *reg,
		           *reg1;

		    reg1 = rouptr;
		    reg = reg1;
		    if (label[0] != EOL) {
			while (reg < rouend) {
			    reg++;
			    j = 0;
			    while (*reg == label[j]) {
				reg++;
				j++;
			    }
			    if (label[j] == EOL) {
				if (*reg == TAB || *reg == SP)
				    goto off;
/* call of procedure without specifying a parameter list */
				if (*reg == '(') {
				    if (dofram0 == 0)
					dofram0 = dofrmptr;
				    goto off;
				}
			    }
			    reg = (reg1 = reg1 + UNSIGN (*reg1) + 2);

			}
			{
			    ierr = LBLUNDEF;
			    stcpy (varerr, label);	/* to be included in error message */
			    if (dofram0)
				dofrmptr = dofram0;	/* reset frame pointer */
			    zload (rou_name);
			    errex = TRUE;
			    goto errexfun;
			}
		    }
		  off:
		    roucu0 = reg1;
		}
		if (roucu0 >= rouend) {
		    ierr = LBLUNDEF;
		    stcpy (varerr, label);	/* to be included in error message */
		    if (dofram0)
			dofrmptr = dofram0;	/* reset frame pointer */
		    zload (rou_name);
		    errex = TRUE;
		    goto errexfun;
		}
		if (routine[0] != EOL)
		    stcpy (rou_name, routine);
		roucu0++;
		forsw = FALSE;

#ifdef DEBUG_NEWSTACK
                printf("Stack PUSH in expr.c!\r\n");
#endif
#ifdef NEWSTACK
                tmp_stack = (stack_level *)calloc(1,sizeof(stack_level));
                if (!tmp_stack) { ierr = STKOV; goto errexfun; }
                tmp_stack->command = '$';
#ifdef DEBUG_NEWSTACK
                if(!cmdptr) printf("CMDPTR is ZERO!\r\n");
#endif
                tmp_stack->cmdptr  = cmdptr;
                tmp_stack->rouname = namold;
                tmp_stack->roucur  = rouoldc;
                tmp_stack->level   = level; level = 0;
                tmp_stack->ztrap[0]= EOL;
                tmp_stack->new = NULL; tmp_stack->previous = stack;
                stack = tmp_stack;
#else
		if (++nstx > NESTLEVLS) {
		    nstx--;
		    ierr = STKOV;
		    errex = TRUE;
		    goto errexfun;
		}
		nestc[nstx] = '$';
#ifdef DEBUG_NEWSTACK
                if(!cmdptr) printf("CMDPTR is ZERO!\r\n");
#endif
		nestp[nstx] = cmdptr;
		nestn[nstx] = namold;
		nestr[nstx] = rouoldc;
		nestnew[nstx] = 0;
		nestlt[nstx] = level;
		level = 0;		/* push level ; clr level */
		ztrap[nstx][0] = EOL;
#endif
		cmdptr += stcpy (cmdptr, codptr - 1) + 1;
		roucur = roucu0;

		if (dofram0) {
		    char   *reg,
		           *reg1;

		    reg = roucu0;
		    reg1 = dofram0;
		    while ((ch = (*reg++)) != '(')
			if (ch == SP || ch == TAB || ch == EOL)
			    break;
		    if (ch != '(') {
			ierr = TOOPARA;
			dofrmptr = dofram0;
			errex = TRUE;
#ifdef DEBUG_NEWSTACK
                        printf("Cheesy Stack POP in expr.c\r\n");
#endif
#ifdef NEWSTACK
			tmp_stack = stack; stack = stack->previous;
                        free(tmp_stack);
#else
			nstx--;
#endif
			goto errexfun;
		    }
		    j = 0;
		    if (*reg == ')')
			reg++;
		    else
			while ((ch = (*reg++)) != EOL) {
			    if ((ch == ',' || ch == ')') && j) {
				varnam[j] = EOL;
#if 0
  		printf("01 [nstx] nstx is (%d) in expr.c\r\n",nstx);
  		printf("[nestnew[nstx]] is (%d) in expr.c\r\n",nestnew[nstx]);
  		printf("[newptr] newptr is [");
                for(loop=0; loop<50; loop++) 
                  printf("%c", (newptr[loop] == EOL) ? '!' : newptr[loop]);
                printf("] in expr.c\r\n");
#endif
				if (nestnew[nstx] == 0)
				    nestnew[nstx] = newptr;
				if (reg1 < dofrmptr) {
				    if (*reg1 == DELIM) {	/* call by reference */
					if (stcmp (reg1 + 1, varnam)) {		/* are they different?? */
					    symtab (new_sym, varnam, "");
					    symtab (m_alias, varnam, reg1 + 1);
					}
				    } else {
					symtab (new_sym, varnam, "");	/* call by value */
					symtab (set_sym, varnam, reg1);
				    }
				    reg1 += stlen (reg1) + 1;
				} else
				    symtab (new_sym, varnam, "");
				if (ch == ')')
				    break;
				j = 0;
				continue;
			    }
			    if ((ch >= 'A' && ch <= 'Z') ||
				(ch >= 'a' && ch <= 'z') ||
				(ch >= '0' && ch <= '9' && j) ||
				(ch == '%' && j == 0)) {
				varnam[j++] = ch;
				continue;
			    }
			    ierr = ARGLIST;
			    dofrmptr = dofram0;		/* reset frame pointer */
			    errex = TRUE;
#ifdef NEWSTACK
#ifdef DEBUG_NEWSTACK
			    printf("EXPR.C 02 (Stack POP)\r\n");
#endif
			    tmp_stack = stack; stack = stack->previous;
                            free(tmp_stack);
#else
			    nstx--;
#endif
			    goto errexfun;
			}
		    if (reg1 < dofrmptr) {
			ierr = TOOPARA;
			dofrmptr = dofram0;	/* reset frame pointer */
			errex = TRUE;
#ifdef NEWSTACK
#ifdef DEBUG_NEWSTACK
			    printf("EXPR.C 03 (Stack POP)\r\n");
#endif
  		        tmp_stack = stack; stack = stack->previous;
                        free(tmp_stack);
#else
			nstx--;
#endif
			goto errexfun;
		    }
		    dofrmptr = dofram0;
		}
		xecline (0);
		if (repQUIT) {		/* repeat QUIT */
		    stcpy (code, " V 26:\201");
#ifdef DEBUG_NEWSTACK
                    printf("Trying to get at nstx in expr.c (2)\r\n");
#endif
		    intstr (&code[6], nstx - repQUIT);
		    repQUIT = 0;
		    codptr = code;
		    return;
		}
		stcpy (tmp, argptr);

	      errexfun:;
		mcmnd = savmcmnd;
		setpiece = savsetp;
		setop = savop;
		test = savtest;
		stcpy (varnam, savarnam);
		dofram0 = savdofr;
		argptr = partition;
		a = argptr;
		if (spx > 0) {
		    stcpy0 (argptr, savargs, savlen + 256L);
		    free (savargs);
		}
		arg = savarg;
		stcpy0 ((char *) argstck, savastck, (long) ((arg + 1) * sizeof (char *)));

		free (savastck);
		a = savlen + argptr;
		if (savpart != partition) {	/* autoadjust may have changed that */
		    f = 0;
		    while (f <= arg) {
			if (argstck[f])
			    argstck[f] = argstck[f] - savpart + partition;
			f++;
		    }
		}
		if (errex) {
		    if (zexflag && (ierr == NOPGM || ierr == LBLUNDEF))
			ierr = ILLFUN;
		    return;
		}
		if (ierr != OK && ierr != (OK - CTRLB))
		    return;
		stcpy (a, tmp);
		goto exec;
	    }				/* end of extrinsic function/variable section */
	} else if (((ch = *++codptr) >= 'A' && ch <= 'Z') ||
		   (ch >= 'a' && ch <= 'z')) {
	    if (ch < 'a')
		ch += 32;
	    tmp[0] = SP;
	    tmp[1] = f;
	    tmp[2] = ch;
	    b = &tmp[3];
	    while (((ch = *++codptr) >= 'A' && ch <= 'Z') ||
		   (ch >= 'a' && ch <= 'z'))
		*b++ = ch | 0140;
	    *b++ = SP;
	    *b = EOL;
	    if (ch == '(') {		/* function */
		if (f != 'z') {		/* standard instrinsic functions */
		    if (find (
				 " ascii char data extract find fn fnumber get increment justify length na name next order piece \
query qlength ql qsubscript qs random re reverse select st stack text tr translate view ", tmp)
			== FALSE) {
			ierr = ILLFUN;
			return;
		    }
		    if (f == 'f' && tmp[2] == 'n')
			f = FNUMBER;
		    else if (f == 'q' && tmp[2] == 'l')
			f = QLENGTH;
		    else if (f == 'q' && tmp[2] == 's')
			f = QSUBSCRIPT;
		    else if (f == 'r' && tmp[2] == 'e')
			f = REVERSE;
		    else if (f == 's' && tmp[2] == 't')
			f = SVNstack;
		    else if (f == 't' && tmp[2] == 'r')
			f = TRANSLATE;
		    else if (f == 'n' && tmp[2] == 'a')
			f = 'N';
		} else {
		    if ((find (zfunctions, tmp) == FALSE) &&	/* userdefined intrinsic: process as extrinsic */
			(tmp[2] != 'f' || tmp[3] != SP)) {
			f = stlen (tmp) - 1;
			stcpy (&tmp[f], codptr);
			code[0] = '$';
			code[1] = '^';
			code[2] = '%';
			code[3] = 'Z';
			stcpy (&code[4], &tmp[2]);
			codptr = code;
			f = '$';
			zexflag = TRUE;
			goto extra_fun;
		    }
		    f = tmp[2] - 32;
		    if (tmp[3] == SP) {
			if (f == 'S' && s_fun_flag == FALSE)
			    f = 'o';	/* ZSORT(=$ORDER) instead of ZSYNTAX */
			if (f == 'P' && p_fun_flag == FALSE)
			    f = ZPREVIOUS;	/* ZPREVIOUS instead of ZPIECE */
			if (f == 'D' && d_fun_flag == FALSE)
			    f = ZDATA;	/* ZDATA instead of ZDATE */
			if (f == 'N' && n_fun_flag == FALSE)
			    f = ZNEXT;	/* ZNEXT instead of ZNAME */
		    } else
			switch (f) {
			case 'C':
			    if ((stcmp (" zcrc \201", tmp) == 0) ||
				(stcmp (" zcr \201", tmp) == 0))
				f = ZCRC;
			    break;
			case 'D':
			    if (stcmp (" zdata \201", tmp) == 0)
				f = ZDATA;
			    break;
			case 'H':
			    if (stcmp (" zht \201", tmp) == 0)
				f = ZHTIME;
			    break;
			case 'L':
			    if (stcmp (" zlsd \201", tmp) == 0)
				f = ZLSD;
			    break;
			case 'N':
			    if (stcmp (" znext \201", tmp) == 0)
				f = ZNEXT;
			    break;
			case 'P':
			    if (stcmp (" zprevious \201", tmp) == 0)
				f = ZPREVIOUS;
			case 'S':
			    if (stcmp (" zsort \201", tmp) == 0)
				f = 'o';	/* process $ZSORT as $ORDER */
			}
		}
	    } else {			/* special variable */
		if (f != 'z') {
		    if (find (" ec ecode es estack et etrap device horolog io job key principal quit reference st stack storage sy system test ti timezone tl tlevel tr trollback ", tmp) == FALSE) {
			ierr = ILLFUN;
			return;
		    }
		    if (f == 's') {
			if (tmp[2] == 'y')
			    f = SVNsystem;
			if (tmp[2] == 't')
			    f = SVNstack;
		    }
		    if (f == 'e') {
			if (tmp[2] == 'c')
			    f = SVNecode;
			if (tmp[2] == 's')
			    f = SVNestack;
			if (tmp[2] == 't')
			    f = SVNetrap;
		    }
		    if (f == 't') {
			if (tmp[2] == 'i')
			    f = SVNtimezone;
			if (tmp[2] == 'l')
			    f = SVNtlevel;
			if (tmp[2] == 'r')
			    f = SVNtrollback;
		    }
		} else {
		    if (find (zsvn, tmp) == FALSE) {
			*(--b) = EOL;	/* there's a SPace we don't need */
			f = ' ';	/* user defined svn */
		    } else {
			f = tmp[2] - 32;
			if (f == 'T' && tmp[3] == 'r' &&
			    (tmp[4] == SP || (stcmp (" ztrap \201", tmp) == 0)))
			    f = ZTRAP;
			if (f == 'M') {	/* loadable match */
			    if ((f = tmp[3]) >= 'a' && f <= 'z')
				f -= 32;
			    f -= 64;
			}
		    }
		}
	    }
	}
	if (ch != '(') {		/* 'special variable' */
	    codptr--;
	    if (extyp != STRING && extyp != ARGIND && spx == 0)
		return;

	    if ((argstck[++arg] = a) >= s) {
		char   *bak;

		bak = partition;
		if (getpmore == 0) {
		    ierr = STKOV;
		    return;
		}
		a = a - bak + partition;
		b = b - bak + partition;
	    }
/************* special variable evaluation ************************************/

	    switch (f) {

/* $JOB */
	    case 'j':

		intstr (a, pid);
		goto exec;

/* $IO */
	    case 'i':

		intstr (a, io);
		i = stlen (a);
		a[i++] = ':';
		a[i++] = '"';
		i += stcpy (&a[i], dev[io]);
		a[i++] = '"';
		a[i] = EOL;
		goto exec;

/* $PRINCIPAL */
	    case 'p':

		a[0] = '0';
		a[1] = ':';
		a[2] = '"';
		i = 3 + stcpy (&a[3], dev[HOME]);
		a[i++] = '"';
		a[i] = EOL;
		goto exec;

/* $QUIT */
	    case 'q':
#ifdef NEWSTACK
                if(!stack) { a[0] = '0'; }
                else a[0] = '0' | (stack->command == '$');
#else
		a[0] = '0' | (nestc[nstx] == '$');
#endif
		a[1] = EOL;
		goto exec;

/* $TEST */
	    case 't':
		a[0] = '0' | test;
		a[1] = EOL;
		goto exec;

/* $HOROLOG */
	    case 'h':
		{
		    unsigned long ilong,
		            ilong1;

		    ilong1 = time (0L) + tzoffset;	/* make $H local time */
		    ilong = ilong1 / 86400;
		    lintstr (a, ilong + 47117);
		    i = stlen (a);
		    a[i++] = ',';
		    ilong = ilong1 - (ilong * 86400);
		    lintstr (&a[i], ilong);
		    goto exec;
		}
/* $ZHOROLOG() */
	    case 'H':
		{
		    unsigned long ilong,
		            ilong1;
#ifdef USE_GETTIMEOFDAY
		    struct timeval timebuffer;
		    gettimeofday (&timebuffer, NULL);
		    ilong1 = timebuffer.tv_sec + tzoffset;	/* make $ZH local time */
#else
		    struct timeb timebuffer;
		    ftime (&timebuffer);
		    ilong1 = timebuffer.time + tzoffset;	/* make $ZH local time */
#endif
		    ilong = ilong1 / 86400;
		    lintstr (a, ilong + 47117);
		    i = stlen (a);
		    a[i++] = ',';
		    ilong = ilong1 - (ilong * 86400);
		    lintstr (&a[i], ilong);

#ifdef USE_GETTIMEOFDAY
		    if ((ilong = timebuffer.tv_usec)) {
#else
		    if ((ilong = timebuffer.millitm)) {
#endif
			i = stlen (a);
			a[i++] = '.';
			a[i++] = ilong / 100 + '0';
			if (ilong % 100) {
			    a[i++] = ilong % 100 / 10 + '0';
			    if (ilong % 10) {
				a[i++] = ilong % 10 + '0';
			    }
			}
			a[i] = EOL;
		    }
		}
		goto exec;

	    case SVNsystem:
		{
		    int i;
	            stcpy(a, MDC_VENDOR_ID);
		    i = stlen(a);
		    a[i++] = ',';
		    intstr (&a[i], pid);
		}
		goto exec;

	    case SVNtimezone:
		lintstr (a, tzoffset);
		goto exec;

	    case SVNtlevel:
		a[0] = '0';
		a[1] = EOL;
		goto exec;

	    case SVNtrollback:
		a[0] = '0';
		a[1] = EOL;
		goto exec;

	    case SVNecode:
		a[0] = EOL;
		goto exec;

	    case SVNestack:
		a[0] = EOL;
		goto exec;

	    case SVNetrap:
		a[0] = EOL;
		goto exec;

	    case SVNstack:
		a[0] = EOL;
		goto exec;

/* $KEY */
	    case 'k':
		stcpy (a, zb);
		if (*a >= SP && *a < DEL)
		    *a = EOL;
		goto exec;
/* $DEVICE */
	    case 'd':
		*a = EOL;
		goto exec;
/* $STORAGE */
	    case 's':
		lintstr (a, symlen - (rouend - rouptr));
		goto exec;

/* $X */
	    case 'x':

		intstr (a, xpos[io]);
		goto exec;

/* $Y */
	    case 'y':

		intstr (a, ypos[io]);
		goto exec;

/* non-standard special variables */

/* $ZA - on HOME device dummy, else byte offset to begin of file */
	    case 'A':
		if (io == HOME) {
		    a[0] = '0';
		    a[1] = EOL;
		} else {
		    lintstr (a, ftell (opnfile[io]));
		}
		goto exec;

/* $ZB - last keystroke */
	    case 'B':
		stcpy (a, zb);
		goto exec;

/* $ZCONTROLC flag */
	    case 'C':
		a[0] = '0' | zcc;
		zcc = FALSE;
		a[1] = EOL;
		goto exec;

/* $ZERROR */
	    case 'E':
		stcpy (a, zerror);
		goto exec;

/* $ZTRAP */
	    case ZTRAP:
#ifdef NEWSTACK
                stcpy (a, stack->ztrap);
#else
		stcpy (a, ztrap[nstx]);
#endif
		goto exec;

/* $ZPRECISION */
	    case 'P':
		intstr (a, zprecise);
		goto exec;

/* $ZSYSTEM */
	    case 'S':
		intstr (a, zsystem);
		goto exec;

/* $ZVERSION */
	    case 'V':
		stcpy (&a[stcpy (a, "FreeM Version \201")], FREEM_VERSION_STR);
		goto exec;

/* $ZNAME */
	    case 'N':
		i = 0;
		while ((a[i] = rou_name[i]) != EOL) {
		    if (rou_name[i] == '.')
			break;
		    i++;
		}
		a[i] = EOL;
		goto exec;

/* $ZI, INTERRUPT ENABLE/DISABLE */
	    case 'I':
		a[0] = '0' | breakon;
		a[1] = EOL;
		goto exec;

/* $ZDATE */
	    case 'D':
		{
		    unsigned long ilong;

		    ilong = time (0L);
		    ilong = (ilong + tzoffset) / 86400L + 47117L;
		    zdate (a, ilong, datetype);
		}
		goto exec;

/* ZTIME */
	    case 'T':
		{
		    unsigned long ilong;

		    ilong = time (0L);
		    ilong = (ilong + tzoffset) % 86400L;
		    ztime (a, ilong, timetype);
		}
		goto exec;

/* $ZJOB - value of JOB number (of father process) */
	    case 'J':
		intstr (a, father ? father : pid);
		goto exec;

/* $ZORDER - value of physically next global reference @$ZO(@$ZR) */
	    case 'O':
		global  (getnext, tmp, a);

		if (ierr > 0)
		    return;
		goto exec;

/* $ZLOCAL - last local reference */
	    case 'L':
		zname (a, zloc);
		if (ierr > OK)
		    return;
		goto exec;

/* $(Z)REFERENCE - last global reference */
	    case 'r':
	    case 'R':
		zname (a, zref);
		if (ierr > OK)
		    return;
		goto exec;

	    case 'C' - 64:
		stcpy (a, zmc);
		goto exec;		/* loadable match 'controls' */
	    case 'N' - 64:
		stcpy (a, zmn);
		goto exec;		/* loadable match 'numerics' */
	    case 'P' - 64:
		stcpy (a, zmp);
		goto exec;		/* loadable match 'punctuation' */
	    case 'A' - 64:
		stcpy (a, zmu);
		stcat (a, zml);
		goto exec;		/* loadable match 'alphabetic' */
	    case 'L' - 64:
		stcpy (a, zml);
		goto exec;		/* loadable match 'lowercase' */
	    case 'U' - 64:
		stcpy (a, zmu);
		goto exec;		/* loadable match 'uppercase' */
	    case 'E' - 64:
		for (i = NUL; i <= DEL; i++)
		    a[i] = i;
		a[i] = EOL;
		goto exec;		/* 'loadable' match 'everything' */
	    case ' ':			/* user defined special variable */
		udfsvn (get_sym, &tmp[2], a);
		if (ierr <= OK)
		    goto exec;
		ierr = OK;
/* if not found in special variable table, process as extrinsic svn */
/* $$^%Z... all uppercase */
		f = 2;
		while ((ch = tmp[f]) != EOL) {
		    if (ch >= 'a' && ch <= 'z')
			ch -= 32;
		    tmp[f++] = ch;
		}
		stcat (tmp, ++codptr);
		code[0] = '$';
		code[1] = '^';
		code[2] = '%';
		code[3] = 'Z';
		stcpy (&code[4], &tmp[2]);
		codptr = code;
		f = '$';
		zexflag = TRUE;
		arg--;
		goto extra_fun;

	    default:
		ierr = ILLFUN;
		return;
	    }
/* end of specialvariable evaluation */
/******************************************************************************/
	}
	if (++spx >= PARDEPTH) {
	    ierr = STKOV;
	    return;
	}
	op_stck[spx] = f;
	op_stck[++spx] = '$';

      text:
	if (*(codptr + 1) != '@') {
	    f = op_stck[spx - 1];
/* f= (spx>0 ? op_stck[spx-1] : 0);
 * if (f) */ switch (f) {
	    case 't':			/* $TEXT is special */

		if ((argstck[++arg] = a) >= s) {
		    char   *bak;

		    bak = partition;
		    if (getpmore == 0) {
			ierr = STKOV;
			return;
		    }
		    a = a - bak + partition;
		    b = b - bak + partition;
		}
		i = 0;
		while ((ch = *++codptr) != EOL) {
		    if (ch == ')')
			break;
		    if (ch == '+') {
			a[i] = EOL;
			if (++spx > PARDEPTH) {
			    ierr = STKOV;
			    return;
			}
			op_stck[spx] = OPERAND;
			goto comma;
		    }
		    if (ch == '^') {
			a[i] = EOL;
			if (++spx > PARDEPTH) {
			    ierr = STKOV;
			    return;
			}
			op_stck[spx] = OPERAND;
			a += i + 1;
			if (i == 0) {
			    a[0] = '1';
			    a[1] = EOL;
			}
/* just routine name: */
/* return first line  */
			else {
			    a[0] = EOL;
			}
			if ((argstck[++arg] = a) >= s) {
			    char   *bak;

			    bak = partition;
			    if (getpmore == 0) {
				ierr = STKOV;
				return;
			    }
			    a = a - bak + partition;
			    b = b - bak + partition;
			}
			if ((spx + 2) > PARDEPTH) {
			    ierr = STKOV;
			    return;
			}
			op_stck[++spx] = '$';
			op_stck[++spx] = OPERAND;
			goto uparrow;
		    }
		    if ((ch < '0' && ch != '%')		/* illegal character in $TEXT */
			||ch > 'z' ||
			(ch < 'A' && ch > '9') ||
			(ch < 'a' && ch > 'Z')) {
			ierr = INVREF;
			return;
		    }
		    a[i++] = ch;
		}
		a[i] = EOL;
		codptr--;
		goto exec;

	    case 'd':			/* $data() */
	    case 'o':			/* $order() */
	    case 'g':			/* $get() */
	    case 'n':			/* $next() */
	    case 'q':			/* $query() */
	    case 'O':			/* $zorder() */
	    case 'N':			/* $zname() */
	    case ZNEXT:		/* $znext() */
	    case ZPREVIOUS:		/* $zprevious() */
		{
		    if ((ch = *++codptr) >= 'A' && ch <= 'Z')
			goto scan_name;
		    if (ch >= 'a' && ch <= 'z')
			goto scan_name;
		    if (ch == '%' || ch == '^')
			goto scan_name;
		    ierr = INVEXPR;
		    return;
		}
	    }
	}
	codptr++;
	goto nextchr;

    case ':':
/* colon: $select or delimiter */
	if (spx < 2 || op_stck[spx - 2] != 's') {
	    if (op_stck[1] == OPERAND && spx == 1)
		return;
	    ierr = INVEXPR;
	    return;
	}
	arg--;
	spx--;
	if (tvexpr (a) == FALSE) {	/* skip next expr */
	    i = 0;			/* quote */
	    j = 0;			/* bracket */
	    for (;;)
	    {
		ch = *++codptr;
		if (ch == '"') {
		    toggle (i);
		    continue;
		}
		if (i) {
		    if (ch != EOL)
			continue;
		    ierr = QUOTER;
		    return;
		}
		if (ch == ',' && !j) {
		    codptr++;
		    goto nextchr;
		}
		if (ch == '(') {
		    j++;
		    continue;
		}
		if (ch == ')') {
		    if (j--)
			continue;
		    ierr = SELER;
		    return;
		}
		if (ch == EOL) {
		    ierr = SELER;
		    return;
		}
	    }
	}
	codptr++;
	goto nextchr;

    }

  m_operator:

    if (extyp == ARGIND && spx == 1 /* && op_stck[2]!='(' */ )
	return;

    f = op_stck[spx];

    if (++spx > PARDEPTH) {
	ierr = STKOV;
	return;
    }
  op10:;				/* entry for shortcut if first operator */
/* check for NOT_OPERATOR */
    if (ch == NOT)
	if (((ch = *++codptr) == '=' || ch == '<' || ch == '>' || ch == '?' ||
	     ch == '&' || ch == '!' || ch == '[' || ch == ']')) {
	    if (ch == ']' && *(codptr + 1) == ch) {
		codptr++;
		ch = SORTSAFTER;
		if (*(codptr+1)=='=') { 
		    codptr++; 
		    ch=EQSORTS; 
		}
	    }
            if (ch == ']' && *(codptr + 1) == '=') {
                codptr++;
                ch = EQFOLLOWS;
            }
            if (ch == '!' && *(codptr + 1) == ch) {
                codptr++;
                ch = XOR;
            } 
	    op_stck[spx] = SETBIT (ch);
	    if (ch == '?')
		goto scan_pattern;
/*                     a+=stlen(a)+1; */
/* djw: does the while loop do the same as the commented out line above? */
/*      we should decide yes or no and get rid of the other code... */
            while (*a++ != EOL) ;
	    codptr++;
	    goto nextchr;
	} else {
	    op_stck[spx] = NOT;
	    goto nextchr;
	}
    if (ch == '*' && *(codptr + 1) == ch) {
	codptr++;
	ch = POWER;
    }
    if (ch == ']' && *(codptr + 1) == ch) {
	codptr++;
	ch = SORTSAFTER;
    }
    if (ch == '<' && *(codptr + 1) == '=') {
	codptr++;
	ch = SETBIT ('>');
    }
    if (ch == '>' && *(codptr + 1) == '=') {
	codptr++;
	ch = SETBIT ('<');
    }
    if (ch == ']' && *(codptr + 1) == '=') {
	codptr++;
	ch = EQFOLLOWS;
    }
    if (ch == SORTSAFTER && *(codptr + 1) == '=') {
	codptr++;
	ch = EQSORTS;
    }
    if (ch == '$')
	ch = MAXOP;
    if (ch == '^') {
	codptr--;
	return;
    }
    if ((op_stck[spx] = ch) != PATTERN) {
	if (f == OPERAND)		/* binary operator */
/*  a+=stlen(a)+1;  */
	    while (*a++ != EOL) ;
	codptr++;
	goto nextchr;
    }
  scan_pattern:
    if ((ch = *++codptr) == INDIRECT) {	/*  a+=stlen(a)+1;  */
	while (*a++ != EOL) ;
	goto m_operator;
    }
    if ((ch > '9' || ch < '0') && (ch != '.')) {
	ierr = INVEXPR;
	return;
    }
    tmp[0] = ch;
    i = 1;
    f = '1';				/* 'previous' character */
    j = 0;				/* point flag */
    group = 0;				/* grouped pattern match */
    while ((ch = *++codptr) != EOL) {
	if ((ch >= '0') && (ch <= '9')) {
	    tmp[i++] = ch;
	    f = '1';
	    continue;
	}
	if (ch == '.') {
	    if (j) {
		ierr = INVEXPR;
		return;
	    }
	    j++;
	    tmp[i++] = ch;
	    f = '1';
	    continue;
	}
	j = 0;
	if (ch == NOT) {		/* negation of pattern class ? */
	    ch = *(codptr + 1);
	    if ((ch == '"') || (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) {
		tmp[i++] = NOT;
	    } else
		ch = NOT;
	}
	if (ch == '"') {
	    if (f != '1' && f != 'A') {
		ierr = INVEXPR;
		return;
	    }
	    for (;;)
	    {
		tmp[i++] = ch;
		if ((ch = *++codptr) == EOL) {
		    ierr = QUOTER;
		    return;
		}
		if (ch == '"') {
		    if ((f = *(codptr + 1)) != '"') {
			ch = DELIM;
			break;
		    }
		    codptr++;
		}
	    }
	    tmp[i++] = ch;
	    f = '"';
	    continue;
	}
	if (ch == '(') {
	    if (f != '1') {
		ierr = INVEXPR;
		return;
	    }
	    group++;
	    f = '(';
	    tmp[i++] = ch;
	    continue;
	}
	if (group && (ch == ',' || ch == ')')) {
	    if ((f == '1') || (f == '(')) {
		ierr = INVEXPR;
		return;
	    }
	    if (ch == ',') {
		f = '(';
		tmp[i++] = ch;
		continue;
	    }
	    if (ch == ')') {
		group--;
		tmp[i++] = ch;
		continue;
	    }
	}
	if (ch >= 'A' && ch <= 'Z')
	    ch += 32;			/* lower case conversion */
	if (ch == 'z') {		/* loadable match, store as uppercase chars */
	    if (standard) {
		ierr = NOSTAND;
		return;
	    }
	    ch = *++codptr;
	    if (ch == '"') {
		if (f != '1') {
		    ierr = INVEXPR;
		    return;
		}
		codptr--;
		tmp[i++] = 'z';
		continue;
	    }
	    if (ch == '(') {
		if (f != '1') {
		    ierr = INVEXPR;
		    return;
		}
		codptr--;
		continue;
	    }
	    if (ch >= 'A' && ch <= 'Z')
		ch += 32;		/* lower case conversion */
	    if (ch != 'e')
		j = 1;			/* process 'ze' as 'e' */
	}
	if (ch != 'c' &&
	    ch != 'n' &&
	    ch != 'p' &&
	    ch != 'a' &&
	    ch != 'l' &&
	    ch != 'u' &&
	    ch != 'e')
	    break;

	if ((f != '1') && (f != 'A')) {
	    ierr = INVEXPR;
	    return;
	}
	if (j) {
	    ch -= 32;
	    j = 0;
	}
	tmp[i++] = ch;
	f = 'A';

    }
    if ((f == '1') || group) {
	ierr = INVEXPR;
	return;
    }
    tmp[i] = EOL;
    if ((*a = pattern (a, tmp)) > '1') {
	ierr = INVEXPR;
	return;
    }
    if (UNSIGN (op_stck[spx--]) & 0200)
	toggle (*a);
    *(a + 1) = EOL;
    goto next10;

/* process values on stack */

  exec:
    if (spx == 0) {
	if ((ch = *++codptr) == EOL || ch == SP ||
	    ch == ',' || ch == ':' ||
	    (ch == '^' && (extyp == LABEL || extyp == OFFSET)))
	    return;
	op_stck[++spx] = OPERAND;
	goto next10;
    }
    f = op_stck[spx];
    if (f == ARRAY ||
	f == '(') {
	if (++spx > PARDEPTH) {
	    ierr = STKOV;
	    return;
	}
	op_stck[spx] = OPERAND;
	codptr++;
	goto nextchr;
    }
/* process operators */

  nxt_expr:
    if (f == '$') {			/* push 'OPERAND' on stack */
	op_stck[++spx] = OPERAND;
	codptr++;
	goto nextchr;
    }
    if (f == OPERAND) {
	ierr = MISSOP;
	return;
    }
    if (op_stck[--spx] == OPERAND) {	/* binary operators */
	b = a;
	a = argstck[--arg];

	switch (f & 0177) {		/* binary operators, NOT OMITTED */

	case PLUS:

	    stcpy (tmp, b);
	  plus01:
	    atyp = numlit (a);
	    btyp = numlit (tmp);
#ifdef EUR2DEM
	    if (atyp != btyp) {
		char    tmp2[256];

		if ((atyp == 0) && (a[0] == '0'))
		    atyp = btyp;	/* zero is any currency */
		if ((btyp == 0) && (tmp[0] == '0'))
		    btyp = atyp;	/* zero is any currency */
		if (atyp && btyp) {
		    if (atyp > 1) {
			stcpy (tmp2, EUR2WHR[atyp]);
			mul (tmp, tmp2);
		    }
		    if (btyp > 1) {
			zprecise += 4;
			stcpy (tmp2, EUR2WHR[btyp]);
			div (tmp, tmp2, '/');
			zprecise -= 4;
		    }
		} else if (atyp != btyp && typemmflag) {
		    ierr = TYPEMISMATCH;
		    return;
		}
	    }
#endif /* EUR2DEM */
	    add (a, tmp);
	  plus02:
#ifdef EUR2DEM
	    if (atyp == 0)
		goto next05;
	    if (atyp != btyp)
		cond_round (a, zprecise + 2);
	    stcat (a, WHR[atyp]);
#endif /* EUR2EUR */
	    goto next05;

	case MINUS:

	    tmp[0] = '-';
	    stcpy (&tmp[1], b);
	    goto plus01;

	case MULTIPLY:

	    stcpy (tmp, b);
	    atyp = numlit (a);
	    btyp = numlit (tmp);
#ifdef EUR2DEM
	    if (btyp && (atyp == 0)) {
		atyp = btyp;
		btyp = 0;
	    }
	    if (atyp && btyp) {
		if (typemmflag) {
		    ierr = TYPEMISMATCH;
		    return;
		}
		atyp = btyp = 0;
	    }
#endif /* EUR2DEM */
	    mul (a, tmp);
#ifdef EUR2DEM
	    if (atyp == 0)
		goto next05;

	    cond_round (a, zprecise + 2);
	    stcat (a, WHR[atyp]);
#endif /* EUR2DEM */
	    goto next05;

	case DIVIDE:
	case INTDIVIDE:
	case MODULO:

	    stcpy (tmp, b);
	    atyp = numlit (a);
	    btyp = numlit (tmp);
#ifdef EUR2DEM
	    if (atyp != btyp) {
		char    tmp2[256];

		if (atyp && btyp) {
		    if (f == MODULO) {
			if (atyp > 1) {
			    stcpy (tmp2, EUR2WHR[atyp]);
			    mul (tmp, tmp2);
			}
			if (btyp > 1) {
			    stcpy (tmp2, EUR2WHR[btyp]);
			    div (tmp, tmp2, '/');
			}
		    } else {
			if (atyp > 1) {
			    stcpy (tmp2, EUR2WHR[atyp]);
			    mul (tmp, tmp2);
			}
			if (btyp > 1) {
			    stcpy (tmp2, EUR2WHR[btyp]);
			    mul (a, tmp2);
			}
			atyp = btyp = 0;
		    }
		} else if (btyp && typemmflag && (*a != '0' || f == MODULO)) {
		    ierr = TYPEMISMATCH;
		    return;
		}
	    } else if (f != MODULO)
		atyp = 0;
#endif /* EUR2DEM */
	    if (tmp[0] == '0') {
		ierr = DIVER;
		return;
	    }
	    if (atyp != btyp)
		zprecise += 4;
	    div (a, tmp, f);
	    if (atyp != btyp)
		zprecise -= 4;
	    goto plus02;

	case CONCATENATE:

	    if (stcat (a, b))
		goto next05;
	    ierr = MXSTR;
	    return;

	case EQUAL:

	    if (stcmp (a, b))
		*a = '0';
	    else
		*a = '1';
	/* common entry point to reverse the logical value */
	/* of current expression			   */
	notop:
	    if (f & 0200)
		toggle (*a);		/* NOT_OPERAND */
	    a[1] = EOL;
	    goto next05;

	case GREATER:

	    stcpy (tmp, b);
	    atyp = numlit (a);
	    btyp = numlit (tmp);
#ifdef EUR2DEM
	    if (atyp != btyp) {
		char    tmp2[256];

		if ((atyp == 0) && (a[0] == '0'))
		    atyp = btyp;	/* zero is any currency */
		if ((btyp == 0) && (tmp[0] == '0'))
		    btyp = atyp;	/* zero is any currency */
		if (atyp && btyp) {
		    if (atyp > 1) {
			stcpy (tmp2, EUR2WHR[atyp]);
			mul (tmp, tmp2);
		    }
		    if (btyp > 1) {
			stcpy (tmp2, EUR2WHR[btyp]);
			mul (a, tmp2);
		    }
		    cond_round (a, zprecise + 2);
		    cond_round (tmp, zprecise + 2);
		} else if (atyp != btyp && typemmflag) {
		    ierr = TYPEMISMATCH;
		    return;
		}
	    }
#endif /* EUR2DEM */
	    if (comp (tmp, a))
		*a = '1';
	    else
		*a = '0';
	    goto notop;
	case LESS:

	    stcpy (tmp, b);
	    atyp = numlit (a);
	    btyp = numlit (tmp);
#ifdef EUR2DEM
	    if (atyp != btyp) {
		char    tmp2[256];

		if ((atyp == 0) && (a[0] == '0'))
		    atyp = btyp;	/* zero is any currency */
		if ((btyp == 0) && (tmp[0] == '0'))
		    btyp = atyp;	/* zero is any currency */
		if (atyp && btyp) {
		    if (atyp > 1) {
			stcpy (tmp2, EUR2WHR[atyp]);
			mul (tmp, tmp2);
		    }
		    if (btyp > 1) {
			stcpy (tmp2, EUR2WHR[btyp]);
			mul (a, tmp2);
		    }
		    cond_round (a, zprecise + 2);
		    cond_round (tmp, zprecise + 2);
		} else if (atyp != btyp && typemmflag) {
		    ierr = TYPEMISMATCH;
		    return;
		}
	    }
#endif /* EUR2DEM */
	    if (comp (a, tmp))
		*a = '1';
	    else
		*a = '0';
	    goto notop;

	case AND:

	    if (tvexpr (a)) {
		tvexpr (b);
		*a = *b;
	    }
	    goto notop;

	case OR:

	    ch = tvexpr (b);		/* beware case of a="" */
	    if (tvexpr (a) == FALSE && ch)
		*a = '1';
	    goto notop;

        case XOR:
            ch = tvexpr (b);            /* beware case of a="" */
            *a = (tvexpr(a) == ch) ? '0' : '1';
            goto notop;
                            
	case CONTAINS:

	    if (*b == EOL || find (a, b))
		*a = '1';
	    else
		*a = '0';
	    goto notop;		
	    
	case EQFOLLOWS:
	    if (stcmp (a, b) == 0) {
		a[0] = '1';
	        goto notop;
	    }
	case FOLLOWS:

	    if (*b == EOL) {
		if (*a == EOL)
		    *a = '0';
		else
		    *a = '1';
	    }
/* frequent special case */
	    else if (stcmp (a, b) <= 0)
		*a = '0';
	    else
		*a = '1';
	    goto notop;
	    
	case POWER:

	    stcpy (tmp, b);
	    numlit (a);
	    numlit (tmp);
	    power (a, tmp);
	    goto next05;

	case EQSORTS:
	    if (stcmp (a, b) == 0) {
		a[0] = '1';
 	        goto notop;
	    }
	case SORTSAFTER:

	    if (collate (b, a))
		*a = '1';
	    else
		*a = '0';
	    goto notop;

	case MAXOP:

#ifdef NOSCRAMBL
	    if (standard) {
		ierr = NOSTAND;
		return;
	    }
#endif /* NOSCRAMBL */
	    stcpy (tmp, b);
	    numlit (tmp);
	    numlit (a);
	    if (comp (a, tmp))
		stcpy (a, tmp);
	    goto next05;

	case MINOP:

#ifdef NOSCRAMBL
	    if (standard) {
		ierr = NOSTAND;
		return;
	    }
#endif /* NOSCRAMBL */
	    stcpy (tmp, b);
	    numlit (tmp);
	    numlit (a);
	    if (comp (a, tmp) == 0)
		stcpy (a, tmp);
	    goto next05;

	default:
	    ierr = ILLOP;
	    return;
	}
    }					/* end binary operators */
    switch (f) {
    case INDIRECT:
      indirect:
	if (*++codptr == '@' && *(codptr + 1) == '(') {
	    if (a[stlen (a) - 1] == ')') {
		codptr += 2;
		a[stlen (a) - 1] = ',';
	    } else
		codptr++;
	}
	stcpy (a + stlen (a), codptr);
	stcpy (&code[1], a);
	codptr = code;
	*codptr = SP;
	arg--;
	if (spx <= 0) {
	    op_stck[0] = 0;
	    codptr++;
	    goto nextchr;
	}
	if ((op_stck[spx] & 0177) != PATTERN)
	    goto text;
	a = argstck[arg];
	goto scan_pattern;

    case MINUS:			/* unary minus */
	b = a + stlen (a) + 1;
	while (b > a) {
	    *b = *(b - 1);
	    b--;
	}
	*a = '-';

    case PLUS:				/* unary plus */

	atyp = numlit (a);
#ifdef EUR2DEM
	if (atyp)
	    stcat (a, WHR[atyp]);
#endif /* EUR2DEM */
	goto nxt_operator;

    case NOT:				/* unary not */

	tvexpr (a);
	toggle (*a);
	goto nxt_operator;

    default:
	ierr = MISSOPD;
	return;
    }					/* end unary operators */

}					/* end expr() */

/******************************************************************************/
	/* $ZSYNTAX */
	/* a simple syntax check.                                    */
	/* $ZSYNTAX expects one argument. If it finds no fault, it   */
	/* returns an empty string. Otherwise it returns a pair of   */
	/* integers separated by a comma. The first number indicates */
	/* the position where the error has been found. The second   */
	/* number returns an error code (same meaning as in $ZE)     */
	/* only the most frequent errors are searched for:           */
	/* - illegal commands                                        */
	/* - not matching brackets                                   */
	/* - not matching quotes                                     */
	/* - missing or surplus arguments                            */
	/* - surplus commata                                         */

void
zsyntax (a)
	char   *a;

{
    register i,
            j,
            f,
            ch;
    char    tmp[256];
    char   *b;
    short   cmnd;
    short   forline;			/* flag: FOR encountered */

    b = a;
    forline = FALSE;
    while ((ch = *b) == '.' || ch == SP)
	b++;				/* level points for blockstr. */
    while ((ch = *b++) != EOL) {	/* scan command */
	if (ch == ';' || ch == '!')
	    break;			/* comment or unix_call */
	if (ch >= 'A' && ch <= 'Z')
	    ch += 32;			/* uppercase to lowercase */
	f = ch;
	cmnd = f;
	if (ch < 'b' || ch > 'z' ||	/* illegal char in cmmd position */
	    ch == 'm' || ch == 't' || ch == 'y') {
	    j = CMMND;
	  zserr:intstr (a, b - a);
	    a[i = stlen (a)] = ',';
	    intstr (&a[++i], j);
	    return;
	}
	i = 1;
	while (((tmp[++i] = ch = *b++) != EOL) &&	/* check full command name */
	       ((ch >= 'A' && ch <= 'Z') ||
		(ch >= 'a' && ch <= 'z')))
	    if (ch < 'a')
		tmp[i] = ch + 32;
	if (f != 'z') {
	    if (i > 2) {
		tmp[0] = SP;
		tmp[1] = f;
		tmp[i] = SP;
		tmp[++i] = EOL;
		if (find (
			     " break close do else for goto hang halt if job kill lock new open quit read set use view write xecute "
			     ,tmp) == FALSE) {
		    j = CMMND;
		    goto zserr;
		}
	    }
	}
	i = 0;				/* quote */
	j = 0;				/* bracket */
	if (ch == ':') {		/*  scan postcond */
	    while ((ch = *b++) != EOL) {
		if (ch == '*' && *b == ch)
		    b++;		/* exponentiation */
                if (ch == '!' && *b == ch)
                    b++;                /* XOR */
		if (ch == ']') {
		    if (*b == ch)
			b++;		/* SORTSAFTER */
		    if (*b == '=')
			b++;		/* EQFOLLOWS or EQSORTS */
		}
		if (ch == '"') {
		    toggle (i);
		    continue;
		}
		if (i)
		    continue;
		if (ch == SP)
		    break;
		if (ch == '$') {
		    ch = *b++;
		    if (ch >= 'A' && ch <= 'Z')
			ch += 32;
		    if ((ch < 'a' || ch > 'z' || ch == 'b' ||
		    ch == 'm' || ch == 'u' || ch == 'w') && ch != '$') {
			j = ILLFUN;
			goto zserr;
		    }
		    if (ch == 's') {	/* $SELECT */
			int     xch,
			        xi,
			        xj;
			char   *xb;
			int     sfl;

			xi = 0;		/* quotes */
			xj = 0;		/* brackets */
			xb = b;		/* do not change old 'b' pointer */
			sfl = TRUE;	/* first ':' expected */
			for (;;)
			{
			    if ((xch = *xb++) == EOL ||
			      ((xch == SP || xch == ',') && xj == 0)) {
				if (xj == 0)
				    break;	/* $STORAGE */
				j = SELER;
				b = xb;
				goto zserr;
			    }
			    if (xch == '"') {
				toggle (xi);
				continue;
			    }
			    if (xi)
				continue;
			    if (xch == ':') {
				if (xj > 1)
				    continue;
				if (sfl) {
				    sfl = FALSE;
				    continue;
				}
				j = SELER;
				b = xb;
				goto zserr;
			    }
			    if (xch == ',') {
				if (xj > 1)
				    continue;
				if (!sfl) {
				    sfl = TRUE;
				    continue;
				}
				j = SELER;
				b = xb;
				goto zserr;
			    }
			    if (xch == '(') {
				xj++;
				continue;
			    }
			    if (xch == ')') {
				if ((xj--) > 1)
				    continue;
				if (sfl) {
				    j = SELER;
				    b = xb;
				    goto zserr;
				}
				break;
			    }
			}
		    }
/* end select check */
		    else if (ch == 'd' ||	/* $DATA */
			     ch == 'g' ||	/* $GET */
			     ch == 'o' ||	/* $ORDER */
			     ch == 'n' ||	/* $NEXT */
			     ch == 'q' ||	/* $QUERY */
			     ch == 'i') {	/* $INCREMENT */
			int     xch,
			        xi,
			        xj;
			char   *xb;

			xb = b;		/* do not change old 'b' pointer */
/* skip name */
			while (((xch = (*xb)) >= 'A' && xch <= 'Z') ||
			       (xch >= 'a' && xch <= 'z'))
			    xb++;
			if (xch == '(') {
			    if ((xch = (*++xb)) == '^' || xch == '%' ||
				(xch >= 'A' && xch <= 'Z') ||
				(xch >= 'a' && xch <= 'z')) {
				xi = xch;
				if (xch == '^' && *(xb + 1) == '%')
				    xb++;
				while
					(((xch = (*++xb)) >= 'A' && xch <= 'Z') ||
					 (xch >= 'a' && xch <= 'z') ||
					 (xch >= '0' && xch <= '9') ||
					 (xch == '.') ||
					 (xch == '/' && xi <= '^') ||
				    (xch == '%' && *(xb - 1) == '/')) ;
			    } else {
				if (xch == '@')
				    continue;
				j = INVEXPR;
				b = xb;
				goto zserr;
			    }
			    xi = 0;	/* quotes */
			    xj = 0;	/* brackets */
			    for (;;)
			    {
				xch = *xb++;
				if (xch == '"' && xj) {
				    toggle (xi);
				    continue;
				}
				if (xi && (xch != EOL))
				    continue;
				if (xch == '(') {
				    xj++;
				    continue;
				}
				if (xch == ')') {
				    if (xj-- > 0)
					continue;
				    break;
				}
				if (xj && xch != EOL)
				    continue;
				if (xch == ',' &&
				 (ch == 'g' || ch == 'q' || ch == 'o'))
				    break;
				j = INVEXPR;
				b = xb;
				goto zserr;
			    }
			}
		    }			/* end data/order/query check */
		    if (ch == 'e' ||	/* $EXTRACT */
			ch == 'p' ||	/* $PIECE */
			ch == 'a' ||	/* $ASCII */
			ch == 'g' ||	/* $GET */
			ch == 'j' ||	/* $JUSTIFY */
			ch == 'l' ||	/* $LENGTH */
			ch == 'r' ||	/* $RANDOM/REVERSE */
			ch == 't' ||	/* $TEXT/TRANSLATE */
			ch == 'f') {	/* $FIND/FNUMBER */
			int     xch,
			        xi,
			        xj,
			        xa;
			char   *xb;

			xb = b;		/* do not change old 'b' pointer */
/* skip name */
			while (((xch = (*xb)) >= 'A' && xch <= 'Z') ||
			       (xch >= 'a' && xch <= 'z'))
			    xb++;
			if (xch == '(') {
			    xi = 0;	/* quotes */
			    xj = 0;	/* brackets */
			    xa = 1;
			    for (;;)
			    {
				xch = (*++xb);
				if (xch == EOL)
				    break;
				if (xch == '"') {
				    toggle (xi);
				    continue;
				}
				if (xi)
				    continue;
				if (xch == '(') {
				    xj++;
				    continue;
				}
				if (xch == ')') {
				    if (xj-- > 0)
					continue;
				    break;
				}
				if (xj == 0 && xch == ',') {
				    xa++;
				    continue;
				}
			    }
			    if ((ch == 'e' && (xa > 3)) ||	/* $EXTRACT */
				(ch == 'p' && (xa < 2 || xa > 4)) ||	/* $PIECE */
				(ch == 'a' && (xa > 2)) ||	/* $ASCII */
				(ch == 'g' && (xa > 2)) ||	/* $GET */
				(ch == 'j' && (xa < 2 || xa > 3)) ||	/* $JUSTIFY */
				(ch == 'l' && (xa > 2)) ||	/* $LENGTH */
				(ch == 'r' && (xa > 1)) ||	/* $RANDON/$REVERSE */
				(ch == 't' && (xa > 3)) ||	/* $TEXT/TRANSLATE */
				(ch == 'f' && (xa < 2 || xa > 3))) {	/* $FIND/FNUMBER */
				j = FUNARG;
				b = xb;
				goto zserr;
			    }
			}
		    }			/* end number of args check */
		    continue;
		}
		if (ch == '(') {
		    j++;
		    continue;
		}
		if (ch == ')') {
		    if (j--)
			continue;
		    break;
		}
		if (ch == ',') {
		    if ((ch = *b) == SP || ch == EOL || ch == ',') {
			j = ARGLIST;
			goto zserr;
		    }
		}
	    }
	    if (i)
		j = QUOTER;
	    else if (j)
		j = j < 0 ? INVEXPR : BRAER;
	    if (j == OK && ch != EOL && ch != SP)
		j = SPACER;
	    if (j)
		goto zserr;
	}				/* end postcond */
	if (ch == SP)
	    ch = *b;
	else if (ch != EOL) {
	    j = SPACER;
	    goto zserr;
	}
	if ((ch == SP || ch == EOL) &&	/* never argumentless */
	    (f == 'j' || f == 'o' || f == 'r' ||
	     f == 's' || f == 'u' || f == 'x' ||
	     f == 'g')) {
	    j = ARGLIST;
	    goto zserr;
	}
/* or.. always argumentless */
	if ((ch != SP && ch != EOL) && (f == 'e' || (f == 'q' && forline))) {
	    j = SPACER;
	    goto zserr;
	}
	if (f == 'f')
	    forline = TRUE;
	if (ch == EOL)
	    break;
/* scan argument */
	i = 0;				/* quotes */
	j = 0;				/* brackets */
	ch = SP;			/* init: previous character */
	for (;;)				/* scan argument */
	{
	    f = ch;			/* f=previous character */
	    if ((ch = *b++) == EOL)
		break;
	    if (ch == '*' && *b == ch)
		b++;			/* exponentiation */
            if (ch == '!' && *b == ch)
                b++;                /* XOR */
	    if (ch == ']') {
		if (*b == ch)
		    b++;		/* SORTSAFTER */
		if (*b == '=')
		    b++;		/* EQFOLLOWS or EQSORTS */
	    }
	    if (ch == '"') {
		toggle (i);
		continue;
	    }
	    if (i)
		continue;
	    if (ch == '$') {
		ch = *b++;
		if (ch >= 'A' && ch <= 'Z')
		    ch += 32;
		if ((ch < 'a' || ch > 'z' || ch == 'b' ||
		   ch == 'm' || ch == 'u' || ch == 'w') && ch != '$') {
		    j = ILLFUN;
		    goto zserr;
		}
		if (ch == 's') {	/* $SELECT */
		    int     xch,
		            xi,
		            xj;
		    char   *xb;
		    int     sfl;

		    xi = 0;		/* quotes */
		    xj = 0;		/* brackets */
		    xb = b;		/* do not change old 'b' pointer */
		    sfl = TRUE;		/* first ':' expected */
		    for (;;)
		    {
			if ((xch = *xb++) == EOL ||
			    ((xch == SP || xch == ',') && xj == 0)) {
			    if (xj == 0)
				break;	/* $STORAGE */
			    j = SELER;
			    b = xb;
			    goto zserr;
			}
			if (xch == '"') {
			    toggle (xi);
			    continue;
			}
			if (xi)
			    continue;
			if (xch == ':') {
			    if (xj > 1)
				continue;
			    if (sfl) {
				sfl = FALSE;
				continue;
			    }
			    j = SELER;
			    b = xb;
			    goto zserr;
			}
			if (xch == ',') {
			    if (xj > 1)
				continue;
			    if (!sfl) {
				sfl = TRUE;
				continue;
			    }
			    j = SELER;
			    b = xb;
			    goto zserr;
			}
			if (xch == '(') {
			    xj++;
			    continue;
			}
			if (xch == ')') {
			    if ((xj--) > 1)
				continue;
			    if (sfl) {
				j = SELER;
				b = xb;
				goto zserr;
			    }
			    break;
			}
		    }
		}
/* end select check */
		else if (ch == 'd' ||	/* $DATA */
			 ch == 'g' ||	/* $GET */
			 ch == 'o' ||	/* $ORDER */
			 ch == 'n' ||	/* $NEXT */
			 ch == 'q') {	/* $QUERY */
		    int     xch,
		            xi,
		            xj;
		    char   *xb;

		    xb = b;		/* do not change old 'b' pointer */
/* skip name */
		    while (((xch = (*xb)) >= 'A' && xch <= 'Z') ||
			   (xch >= 'a' && xch <= 'z'))
			xb++;
		    if (xch == '(') {
			if ((xch = (*++xb)) == '^' || xch == '%' ||
			    (xch >= 'A' && xch <= 'Z') ||
			    (xch >= 'a' && xch <= 'z')) {
			    xi = xch;
			    if (xch == '^' && *(xb + 1) == '%')
				xb++;
			    while
				    (((xch = (*++xb)) >= 'A' && xch <= 'Z') ||
				     (xch >= 'a' && xch <= 'z') ||
				     (xch >= '0' && xch <= '9') ||
				     (xch == '.') ||
				     (xch == '/' && xi <= '^') ||
				     (xch == '%' && *(xb - 1) == '/')) ;

			} else {
			    if (xch == '@')
				continue;
			    j = INVEXPR;
			    b = xb;
			    goto zserr;
			}
			xi = 0;		/* quotes */
			xj = 0;		/* brackets */
			for (;;)
			{
			    xch = *xb++;
			    if (xch == '"' && xj) {
				toggle (xi);
				continue;
			    }
			    if (xi && (xch != EOL))
				continue;
			    if (xch == '(') {
				xj++;
				continue;
			    }
			    if (xch == ')') {
				if (xj-- > 0)
				    continue;
				break;
			    }
			    if (xj && xch != EOL)
				continue;
			    if (xch == ',' &&
				(ch == 'g' || ch == 'q' || ch == 'o'))
				break;
			    j = INVEXPR;
			    b = xb;
			    goto zserr;
			}
		    }
		}			/* end data/order/query check */
		if (ch == 'e' ||	/* $EXTRACT */
		    ch == 'p' ||	/* $PIECE */
		    ch == 'a' ||	/* $ASCII */
		    ch == 'g' ||	/* $GET */
		    ch == 'j' ||	/* $JUSTIFY */
		    ch == 'l' ||	/* $LENGTH */
		    ch == 'r' ||	/* $RANDON/$REVERSE */
		    ch == 't' ||	/* $TEXT/TRANSLATE */
		    ch == 'f') {	/* $FIND/FNUMBER */
		    int     xch,
		            xi,
		            xj,
		            xa;
		    char   *xb;

		    xb = b;		/* do not change old 'b' pointer */
/* skip name */
		    while (((xch = (*xb)) >= 'A' && xch <= 'Z') ||
			   (xch >= 'a' && xch <= 'z'))
			xb++;
		    if (xch == '(') {
			xi = 0;		/* quotes */
			xj = 0;		/* brackets */
			xa = 1;
			for (;;)
			{
			    xch = (*++xb);
			    if (xch == EOL)
				break;
			    if (xch == '"') {
				toggle (xi);
				continue;
			    }
			    if (xi)
				continue;
			    if (xch == '(') {
				xj++;
				continue;
			    }
			    if (xch == ')') {
				if (xj-- > 0)
				    continue;
				break;
			    }
			    if (xj == 0 && xch == ',') {
				xa++;
				continue;
			    }
			}
			if ((ch == 'e' && (xa > 3)) ||	/* $EXTRACT */
			    (ch == 'p' && (xa < 2 || xa > 4)) ||	/* $PIECE */
			    (ch == 'a' && (xa > 2)) ||	/* $ASCII */
			    (ch == 'o' && (xa > 2)) ||	/* $ORDER */
			    (ch == 'q' && (xa > 2)) ||	/* $QUERY */
			    (ch == 'g' && (xa > 2)) ||	/* $GET */
			    (ch == 'j' && (xa < 2 || xa > 3)) ||	/* $JUSTIFY */
			    (ch == 'l' && (xa > 2)) ||	/* $LENGTH */
			    (ch == 't' && (xa > 3)) ||	/* $TEXT/TRANSLATE */
			    (ch == 'f' && (xa < 2 || xa > 3))) {	/* $FIND/FNUMBER */
			    j = FUNARG;
			    b = xb;
			    goto zserr;
			}
		    }
		}			/* end number of args check */
		continue;
	    }
	    if (ch == '(') {
		if (f == ')' || f == '"') {
		    j = ARGLIST;
		    goto zserr;
		}
		j++;
		continue;
	    }
	    if (ch == ')') {
		tmp[0] = f;
		tmp[1] = EOL;
		if (find (" !#&'(*+,-/:<=>?@[\\]_\201", tmp)) {
		    j = MISSOPD;
		    goto zserr;
		}
		if (j--)
		    continue;
		break;
	    }
	    if (ch == SP)
		break;
	    tmp[0] = ch;
	    tmp[1] = EOL;
	    if (ch == '/' && (cmnd == 'r' || cmnd == 'w') && (f == SP || f == ',')) {
		int     xch,
		        xi,
		        xj;
		char   *xb;

		xi = 0;			/* quotes */
		xj = 0;			/* brackets */
		xb = b;			/* do not change old 'b' pointer */
		while ((xch = *xb++) != EOL) {
		    if (xch == '"') {
			toggle (xi);
			continue;
		    }
		    if (xi)
			continue;
		    if (xch == '(') {
			xj++;
			continue;
		    }
		    if (xch == ')') {
			if ((xj--) > 1)
			    continue;
			xch = *xb++;
			break;
		    }
		    if (xj)
			continue;
		    if ((xch < 'A' || xch > 'Z') &&
			(xch < '1' || xch > '3'))
			break;
		}
		if (xch != ',' && xch != SP && xch != EOL) {
		    b = xb;
		    j = SPACER;
		    goto zserr;
		}
		if (--xb == b) {
		    j = ARGLIST;
		    goto zserr;
		}
	    }
	    if (f == '?' && cmnd != 'r' && cmnd != 'w' &&
		find ("@1234567890.\201", tmp) == 0) {	/* pattern match */
		j = MISSOPD;
		goto zserr;
	    }
/* note: write/read may have !?*#/ not as binary op */
	    if (find ("&<=>[\\]_\201", tmp) ||	/* binary operator */
		(find ("!?*#/\201", tmp) && cmnd != 'r' && cmnd != 'w'))
/* some may be negated */
	    {
		if (find ("#*/\\_\201", tmp) || f != NOT) {
		    tmp[0] = f;
		    if (find (" &'(+-<=>[\\]_\201", tmp) ||
			(find ("!?*#/\201", tmp) && cmnd != 'r' && cmnd != 'w')) {
			j = MISSOPD;
			goto zserr;
		    }
		}
		continue;
	    }
	    if (ch == '+' || ch == '-') {
		if (f == NOT) {
		    j = MISSOPD;
		    goto zserr;
		}
		continue;
	    }
	    if (ch == ':') {
		if (f == ',') {
		    j = MISSOPD;
		    goto zserr;
		}
		continue;
	    }
	    if (ch == '`' || ch == ';' || ch == '{' || ch == '|' ||
		ch == '}' || ch == '~') {	/* illegal characters */
		j = ILLOP;
		goto zserr;
	    }
	    if (ch == '$') {		/* check function */
		if (((f = *b | 0140) < 'a' || f > 'z') && f != '$') {
		    j = ILLFUN;
		    goto zserr;
		}
		continue;
	    }
	    if (ch == ',') {		/* comma is a delimiter! */
		if (*(b - 2) == SP || (f = *b) == SP || f == EOL || f == ',') {
		    j = ARGLIST;
		    goto zserr;
		}
	    }
	}
	if (i)
	    j = QUOTER;
	else if (j)
	    j = j > 0 ? INVEXPR : BRAER;
	if (j)
	    goto zserr;
	if (ch == EOL)
	    break;
/* skip spaces before next command */
	while (ch == SP || ch == TAB)
	    ch = *b++;
	b--;
    }
    *a = EOL;				/* no error found */
    return;
}					/* end zsyntax() */

void
zdate (a, htime, type)			/* converts $H type date to string */
	char   *a;			/* result string */
	long    htime;			/* $H type date */
	int     type;			/* type of result string */
{
    long    d,
            m,
            y,
            i,
            j;
    int     lp;

    if ((htime += 672411L) <= 0L) {
	ierr = ARGER;
	return;
    }
    if (htime < datGRbeg[type]) {	/* julian calendar */
	d = htime;
	if (htime > 3285) {		/* 4/8 no leap years */
	    htime++;
	    y = htime / 1461 * 4;
	    htime = htime % 1461;
	    y += htime / 365;
	    lp = (y % 4 == 0);
	    d -= y * 365 + (y - 1) / 4 - 2;
	} else {
	    y = (htime - 1) / 365;
	    d -= y * 365;
	    lp = FALSE;
	}
    } else {				/* gregorian calendar */
	d = htime--;
	y = htime / 146097 * 400;
	htime = htime % 146097;
	y += htime / 36524 * 100;
	htime = htime % 36524;
	y += htime / 1461 * 4;
	htime = htime % 1461;
	y += htime / 365;
	lp = ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0);
	d -= y * 365 + (y - 1) / 4 - (y - 1) / 100 + (y - 1) / 400;
    }
    if (y == 0) {
	ierr = ARGER;
	return;
    }
    m = 1;
    if ((i = 31) >= d)
	goto zd1;
    m++;
    d = d - i;
    i = 28;
    if (lp)
	i++;
    if (i >= d)
	goto zd1;
    m++;
    d -= i;
    if ((i = 31) >= d)
	goto zd1;
    m++;
    d -= i;
    if ((i = 30) >= d)
	goto zd1;
    m++;
    d -= i;
    if ((i = 31) >= d)
	goto zd1;
    m++;
    d -= i;
    if ((i = 30) >= d)
	goto zd1;
    m++;
    d -= i;
    if ((i = 31) >= d)
	goto zd1;
    m++;
    d -= i;
    if (i >= d)
	goto zd1;
    m++;
    d -= i;
    if ((i = 30) >= d)
	goto zd1;
    m++;
    d -= i;
    if ((i = 31) >= d)
	goto zd1;
    m++;
    d -= i;
    if ((i = 30) >= d)
	goto zd1;
    m++;
    d -= i;
    if ((i = 31) >= d)
	goto zd1;
    m++;
    d -= i;
  zd1:;

    i = 0;

    if (dat4flag[type] == 0) {		/* D-M-Y format */
	if (d > 9)
	    a[i++] = d / 10 + '0';
	else if ((a[i++] = dat3char[type]) == EOL)
	    i--;
	a[i++] = d % 10 + '0';
	j = 0;
	while ((a[i++] = dat1char[type][j++]) != EOL) ;
	i--;
	i += stcpy (&a[i], month[type][m - 1]);
	j = 0;
	while ((a[i++] = dat2char[type][j++]) != EOL) ;
	i--;
    } else if (dat4flag[type] == 1) {	/* M-D-Y format */
	i += stcpy (&a[i], month[type][m - 1]);
	j = 0;
	while ((a[i++] = dat1char[type][j++]) != EOL) ;
	i--;
	if (d > 9)
	    a[i++] = d / 10 + '0';
	else if ((a[i++] = dat3char[type]) == EOL)
	    i--;
	a[i++] = d % 10 + '0';
	j = 0;
	while ((a[i++] = dat2char[type][j++]) != EOL) ;
	i--;
    }
/* 2 digits if current century  ??? */

    j = ((((time (0L) + tzoffset) / 86400) > 10956) ? 2000 : 1900);
    if (dat5flag[type] && y >= j && y < (j + 100)) {
	y = y % 100;
	a[i++] = y / 10 + '0';
	a[i++] = y % 10 + '0';
    } else
/* leading zero in ***true*** 2digit years ! */
    {
	if (y > 9 && y < 100 && dat5flag[type])
	    a[i++] = '0';
	j = y;
	if (j > 9999) {
	    if (j > 999999) {
		a[i++] = y / 1000000 + '0';
		y = y % 1000000;
	    }
	    if (j > 99999) {
		a[i++] = y / 100000 + '0';
		y = y % 100000;
	    } {
		a[i++] = y / 10000 + '0';
		y = y % 10000;
	    }
	}
	if (j > 999) {
	    a[i++] = y / 1000 + '0';
	    y = y % 1000;
	}
	if (j > 99) {
	    a[i++] = y / 100 + '0';
	    y = y % 100;
	}
	if (j > 9) {
	    a[i++] = y / 10 + '0';
	}
	a[i++] = y % 10 + '0';
    }

    if (dat4flag[type] == 2) {		/* Y-M-D format */
	j = 0;
	while ((a[i++] = dat1char[type][j++]) != EOL) ;
	i--;
	i += stcpy (&a[i], month[type][m - 1]);
	j = 0;
	while ((a[i++] = dat2char[type][j++]) != EOL) ;
	i--;
	if (d > 9)
	    a[i++] = d / 10 + '0';
	else if ((a[i++] = dat3char[type]) == EOL)
	    i--;
	a[i++] = d % 10 + '0';
    }
    a[i] = EOL;
    return;
}					/* end zdate() */

void
ztime (a, htime, type)			/* converts $H type time to string */
	char   *a;			/* result string */
	long    htime;			/* $H type time */
	int     type;			/* type of result string */
{
    int     i,
            j;

    i = 0;
    if (htime < 0 || htime >= 86400) {
	ierr = ARGER;
	return;
    }
    j = htime / 3600;			/* hours */
    if (tim4flag[type] && (j > 12))
	j -= 12;
    if ((a[0] = (char) (j / 10 + '0')) == '0')
	a[0] = tim3char[type];
    if (a[0] != EOL)
	i = 1;
    a[i++] = (char) (j % 10 + '0');
    if ((a[i] = tim1char[type]) != EOL)
	i++;
    j = htime % 3600 / 60;		/* minutes */
    a[i++] = (char) (j / 10 + '0');
    a[i++] = (char) (j % 10 + '0');
    if (tim5flag[type] == FALSE) {
	if ((a[i] = tim2char[type]) != EOL)
	    i++;
	j = htime % 60;			/* seconds */
	a[i++] = (char) (j / 10 + '0');
	a[i++] = (char) (j % 10 + '0');
    }
    if (tim4flag[type]) {
	a[i++] = SP;
	a[i++] = (htime < 43200 ? 'A' : 'P');
	a[i++] = 'M';
    }
    a[i] = EOL;
    return;
}					/* end ztime() */

void
zkey (a, type)
	char   *a;			/* result string */
	long    type;			/* type of transform */
{
    char    del0,
            del1,
            del2,
            del3,
            del4;
    int     f;
    char    prod_rule[256];
    int     i;
    int     ncs;			/* flag: non_collating_substring */

    if (type == 0)
	type = (-v93);			/* zero is reverse of default type */
    if ((f = (type < 0)))
	type = (-type);
    if (type-- > NO_V93) {
	ierr = ARGER;
	return;
    }
    del2 = v93a[type][0];		/* delimiter between primary/seconary key */
    del0 = v93a[type][1];		/* delimiter between 'from' and 'to' substring */
    del3 = '(';				/* introducer for 'non-collating' substrings */
    del4 = ')';				/* terminator for 'non-collating' substring */
    ncs = FALSE;			/* non_collating_substring flag */
    if (del0 == EOL)
	return;				/* no rule under of this type */
    del1 = v93a[type][2];		/* delimiter between different from/to pairs */
/* production rule, stripped from delimiter declaration */
/* with an added separator character at both ends */
    i = stcpy (prod_rule, &v93a[type][2]);
    prod_rule[i] = del1;
    prod_rule[++i] = EOL;
    if (f)
	goto backw;			/* negative is backward transform */
/* forward transform */
    i = stlen (a);
    if (i == 0)
	return;				/* string empty - nothing to do */
    {
	char    ct0[256],
	        ct1[256];
	int     ch,
	        d,
	        i1,
	        j,
	        n0,
	        n1,
	        pos;
	char    c;

	i = 0;
	n0 = 0;
	n1 = 0;
	while ((c = a[i]) != EOL) {	/* non-collating substring? */
	    if (c == del3) {		/* introducer valid only with matching terminator! */
		j = i;
		while ((ch = a[++j]) != EOL)
		    if (ch == del4)
			break;
		if (ch == del4) {
		    while (i <= j)
			ct1[n1++] = a[i++];
		    continue;
		}
	    }
	    j = 0;
	    d = 0;
/* search for longest matching string */
	    while ((ch = prod_rule[j++]) != EOL)
		if (ch == del1) {
		    if (prod_rule[j] != c)
			continue;
		    i1 = i;
		    while ((ch = prod_rule[j++]) != del0 && ch == a[i1++]) ;
		    if (ch != del0)
			continue;
		    if ((ch = i1 - i) > d) {
			d = ch;
			pos = j;
		    }
		}
	    if (n0 > STRLEN) {
		ierr = MXSTR;
		return;
	    }				/* string too long */
	    if (d == 0) {
		ct0[n0++] = c;
		ct1[n1++] = '0';
		i++;
		continue;
	    }
	    j = 0;
	    c = prod_rule[pos];
	    ch = '0';
	    if (c == del1) {
		ct1[n1++] = ' ';
		while (j <= pos) {
		    if (prod_rule[j] == del0)
			ch++;
		    j++;
		}
	    } else {
		while (j <= pos) {
		    if (prod_rule[j] == del0 && prod_rule[j + 1] == c)
			ch++;
		    j++;
		}
	    }
	    j = 0;
	    i += d;
	    ct1[n1++] = ch;
	    while ((ct0[n0++] = prod_rule[pos++]) != del1) {
		if (n1 > STRLEN) {
		    ierr = MXSTR;
		    return;
		}			/* string too long */
	    }
	    n0--;
	}
	ct0[n0++] = del2;
	ct0[n0] = EOL;
	ct1[n1] = EOL;
/* purge trailing zeroes */
	while (ct1[--n1] == '0') {
	    ct1[n1] = EOL;
	    if (n1 == 0) {
		n0--;
		break;
	    }
	}
	if (n0 + n1 > STRLEN) {
	    ierr = MXSTR;
	    return;
	}				/* string too long */
	stcpy (a, ct0);
	stcpy (&a[n0], ct1);
    }
    return;
/* backward transform */
  backw:;
    i = stlen (a);
    if (i == 0)
	return;				/* string empty */
    {
	int     c,
	        ch,
	        d,
	        n0,
	        n1,
	        n2,
	        j;
	char    z[256];

	stcpy (z, a);
	n0 = 0;
	n1 = 0;
	n2 = 0;
	while ((d = z[n1++]) != EOL && (d != del2)) ;
	if (d == EOL)
	    return;			/* nothing to change */
	for (;;)
	{
	    c = z[n0];
	    d = z[n1];
	    if (c == del2 && d == EOL)
		break;
	    if (d == EOL)
		d = '0';
	    else
		n1++;
	    if (d == del3) {
		a[n2++] = d;
		ncs = TRUE;
		continue;
	    }
	    if (ncs) {
		a[n2++] = d;
		if (d == del4)
		    ncs = FALSE;
		continue;
	    }
	    if (d == ' ') {		/* replacement with no chars */
		d = z[n1++] - '0';
		j = 1;
		while ((ch = prod_rule[j++]) != EOL)
		    if (ch == del0 && (--d) == 0)
			break;
	    } else {
		if ((d -= '0') == 0) {
		    a[n2++] = c;
		    n0++;
		    continue;
		}
		j = 1;
		while ((ch = prod_rule[j++]) != EOL)
		    if (ch == del0 && prod_rule[j] == c && (--d) == 0)
			break;
	    }
	    d = j;
	    while ((ch = prod_rule[j++]) != EOL) {
		if (ch == del1)
		    break;
		n0++;
	    }
	    d--;
	    while (prod_rule[d--] != del1) ;
	    if (prod_rule[d + 2] == EOL) {
		ierr = ARGER;
		return;
	    }				/* string is not of proper format */
	    d++;
	    while ((ch = prod_rule[++d]) != del0)
		a[n2++] = ch;
	}
	a[n2] = EOL;
    }
    return;
}					/* end zkey() */

/* structured system variable management */

void
ssvn (action, key, data)		/* ssvn functions */
	short   action;			/* set_sym      get_sym   */

					/* kill_sym     $data     */
					/* kill_all     $order    */
					/* killexcl     $query    */
					/* new_sym                */
					/* new_all      getinc    */
					/* newexcl                */
					/* killone      m_alias   */
					/* merge_sym    zdata     */

	char   *key,			/* ssvn as ASCII-string */
	       *data;
{
    int     i,
            j;
    char    ch;
    char    tmp[256];

    i = 1;
    j = 2;
    while ((ch = key[j]) != EOL) {
	if (ch >= 'A' && ch <= 'Z')
	    ch += 32;
	if (ch == DELIM)
	    break;
	tmp[i++] = ch;
	j++;
    }
    tmp[0] = SP;
    tmp[i++] = SP;
    tmp[i] = EOL;
    if (tmp[1] != 'z') {
	if (find (" c character d device g global j job l lock r routine s system ", tmp) == FALSE) {
	    ierr = UNDEF;
	    return;
	}
	switch (tmp[1]) {
	case 'c':			/* ^$CHARACTER ssvn */
	    *data = EOL;
	    break;
	case 'd':			/* ^$DEVICE ssvn */
	    *data = EOL;
	    break;
	case 'g':			/* ^$GLOBAL ssvn */
	    *data = EOL;
	    break;
	case 'j':			/* ^$JOB ssvn */
	    *data = EOL;
	    break;
	case 'l':			/* ^$LOCK ssvn */
	    *data = EOL;
	    break;
	case 'r':			/* ^$ROUTINE ssvn */
	    *data = EOL;
	    break;
	case 's':			/* ^$SYSTEM ssvn */
	    *data = EOL;
	    break;
	default:
	    ierr = UNDEF;
	    break;
	}

    } else {				/* implementation specific ssvns */
	ierr = UNDEF;
	return;
    }
    return;
}

int
levenshtein (word1, word2)
	char   *word1,
	       *word2;
{
    int     l1,
            l2,
            i,
            j,
            m,
            t,
            x;
    char    d[2][256];

    l1 = stlen (word1);
    word1--;
    l2 = stlen (word2);
    word2--;
    if (l1 == 0)
	return (l2);
    if (l2 == 0)
	return (l1);
    t = 0;
    for (i = 0; i <= l1; i++)
	d[0][i] = i;
    for (j = 1; j <= l2; j++) {
	t ^= 1;
	d[t][0] = j;
	for (i = 1; i <= l1; i++) {
	    m = d[t ^ 1][i - 1];
	    if (word1[i] != word2[j])
		m++;
	    x = d[t ^ 1][i];
	    if (++x < m)
		m = x;
	    x = d[t][i - 1];
	    if (++x < m)
		m = x;
	    d[t][i] = m;
	}
    }
    return (m);
}

	/* conditional rounding */
	/* 'a' is assumed to be a 'canonic' numeric string           */
	/* it is rounded to 'digits' fractional digits provided that */
	/* the canonic result has at most (digits-2) frac.digits     */
void
cond_round (a, digits)
	char   *a;
	int     digits;
{
    int     ch,
            i,
            point,
            lena;

    point = -1;
    i = 0;
    i = 0;
    while (a[i] != EOL) {
	if (a[i] == '.')
	    point = i;
	i++;
    }
    lena = i;
    if (point < 0)
	point = i;
    if ((point + digits + 1) >= i)
	return;				/* nothing to round */
    i = point + digits + 1;
    if (a[i] < '5') {
	if ((a[i - 1] != '0') || (a[i - 2] != '0'))
	    return;			/* condition! */
	a[i] = EOL;
	while (a[--i] == '0')
	    a[i] = EOL;
	if (a[i] == '.') {
	    a[i] = EOL;
	    if (i == 0 || (i == 1 && a[0] == '-'))
		a[0] = '0';
	}
	return;
    }
    if (a[i - 1] != '9' || a[i - 2] != '9')
	return;				/* condition */
    for (;;)
    {
	if (i >= point)
	    a[i] = EOL;
	else
	    a[i] = '0';
	if (--i < (a[0] == '-')) {
	    for (i = lena; i >= 0; i--)
		a[i + 1] = a[i];
	    a[a[0] == '-'] = '1';
	    break;
	}
	if ((ch = a[i]) == '.')
	    continue;
	if (a[i] < '9' && ch >= '0') {
	    a[i] = ++ch;
	    break;
	}
    }
    return;
}					/* end cond_round */

/* End of $Source: /cvsroot-fuse/gump/FreeM/src/expr.c,v $ */
