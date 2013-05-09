/***
 * $Source: /cvsroot-fuse/gump/FreeM/src/xecline.c,v $
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
 * mumps command line interpreter
 * 
 */

#include "mpsdef.h"
#include <errno.h>
#ifdef USE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#include <sys/types.h>
#include <sys/timeb.h>

/* mumps commands */
#define ABLOCK       0
#define ASSIGN      'a'
#define ASTART       1
#define ASTOP        2
#define AUNBLOCK     3
#define BREAK       'b'
#define CLOSE       'c'
#define DO          'd'
#define DO_BLOCK     4
#define ELSE        'e'
#define ESTART       5
#define ESTOP        6
#define ETRIGGER     7
#define FOR         'f' /* ALSO in symtab.c */
#define GOTO        'g'
#define HA          'h'
#define HALT         8
#define HANG         9
#define IF          'i'
#define JOB         'j'
#define KILL        'k'
#define KVALUE      10 
#define KSUBSC      11
#define LOCK        'l'
#define MERGE       'm'
#define NEWCMD      'n'
#define OPEN        'o'
#define QUIT        'q'
#define READ        'r'
#define RLOAD       12
#define RSAVE       13
#define SET         's'
#define TCOMMIT     14
#define THEN        15
#define TRESTART    16
#define TROLLBACK   17
#define TSTART      18
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

#ifdef NEWSTACK
extern stack_level *stack;
#endif

int
xecline (typ)	/* interprets and xecutes line of code at 'codptr' */
	int     typ;			/* where to GO */
{
    char   *namold;
    long    rouoldc;
    unsigned long jobtime;
    char    label[256],
            routine[256];
    char    vn[65535],
            tmp[256],
            tmp2[256],
            tmp3[256];

    register i;
    register j;
    register ch;
#ifdef DEBUG_NEWSTACK
    int loop;
#endif

#ifdef NEWSTACK
    stack_level *tmp_stack; /* Temporary stack-node storage */
#endif

    if (typ == 1)
	goto restart;
    if (typ == 2)
	goto err;
/*      if (typ==0) goto next_line; */

  next_line:				/* entry point for next command line */

    while ((roucur < rouend) && (ch = (*roucur++)) != TAB && ch != SP)
	;	/* skip label */
    if (roucur >= rouend) goto quit0;	/* end of routine implies QUIT */
    while ((ch = *roucur) == TAB || ch == SP)
	roucur++;

    i = 0;
    if (ch == '.') {			/* get level count */
	do {
	    i++;
	    while ((ch = (*++roucur)) == SP || ch == TAB) ;
	}
	while (ch == '.');
    }
    if (i != level) {
	if (mcmnd == GOTO) {
	    ierr = LVLERR;
	    goto err;
	}
	if (i < level)
	    goto quit0;
	else {
	    roucur += stlen (roucur) + 2;
	    goto next_line;
	}
    }
    i = stcpy (code, roucur) + 1;
    code[i] = EOL;
    roucur += i + 1;
    codptr = code;

  next_cmnd:				/* continue line entry point */

    if (ierr > OK)
	goto err;
  next0:
    do {
	if ((ch = *codptr) == EOL) {
	    if (forsw) goto for_end;
	    goto next_line;
	}
	codptr++;
    } while (ch == SP);

/* decode command word */

    if (ch < 'A') {
	if (ch == ';') {		/* COMMENT */
	    goto skip_line;
	}
	if (ch == '!') {		/* UNIXCALL */
	    if (standard) {
		ierr = NOSTAND;
		goto err;
	    }
	    close_all_globals ();		/* close all globals */
	    set_io (UNIX);
					/* don't catch child dies signal */
	    SIGNAL_ACTION(SIGUSR1, SIG_IGN, NULL);

	    tmp2[stcpy (tmp2, codptr)] = NUL;
	    if (demomode)
		fputc (d1char, stdout);
	    if (tmp2[0] == '!') {	/* call does not return!! *//* close all files, except for stdin etc. */
/* that's not done automatically          */
		j = 1;
		while (j <= MAXDEV) {
		    if (jour_flag && (j == 2)) {
			j++;
			continue;
		    }
		    if (devopen[j])
			fclose (opnfile[j]);
		    devopen[j++] = 0;
		}
		io = HOME;
		lock (" \201", -1, LOCK);	/* un-LOCK */
		lock (" \201", -1, ZDEALLOCATE);	/* ZDEALLOCATE ALL */
		zsystem = execl ("/bin/sh", "sh", "-c", &tmp2[1], 0);
	    } else if (tmp2[0] == '<') {	/* call write output to %-array */
		FILE   *pipdes;
		char    key[STRLEN + 1 /*was 256*/];
		char    data[STRLEN + 1 /*was 256*/];

		vn[0] = '%';
		vn[1] = EOL;
		symtab (kill_sym, vn, data);
		vn[0] = '%';
		vn[1] = EOL;
		data[0] = '0';
		data[1] = EOL;
		symtab (set_sym, vn, data);
		if ((pipdes = popen (&tmp2[1], "r")) == NULL) {
		    zsystem = 1;
		} else {
		    while (fgets (data, STRLEN, pipdes)) {
			vn[0] = '%';
			vn[1] = EOL;
			key[0] = '%';
			key[1] = DELIM;
			symtab (getinc, vn, &key[2]);
			i = strlen (data);
			data[i] = EOL;
			if (i > 1 && data[i - 1] == LF)
			    data[i - 1] = EOL;
			symtab (set_sym, key, data);
			if (ierr == STORE)
			    break;
		    }
		    pclose (pipdes);
		    zsystem = 0;
		}
	    } else if (tmp2[0] == '>') {	/* call read input from %-array */
		FILE   *pipdes;
		char    key[STRLEN + 1/*was 256*/];
		char    data[STRLEN + 1/*was 256*/];
		int     i,
		        k,
		        l;

		vn[0] = '%';
		vn[1] = EOL;
		symtab (get_sym, vn, data);
		ierr = OK;
		k = intexpr (data);
		if (k < 1 || (pipdes = popen (&tmp2[1], "w")) == NULL) {
		    zsystem = 1;
		} else {
		    for (i = 1; i <= k; i++) {
			key[0] = '%';
			key[1] = DELIM;
			intstr (&key[2], i);
			symtab (get_sym, key, data);
			l = stlen (data);
			data[l++] = LF;
			data[l] = NUL;
			fputs (data, pipdes);
		    }
		    pclose (pipdes);
		    zsystem = 0;
		    ierr = OK;
		}
	    } else
		zsystem = system (tmp2);
	    if (demomode)
		fputc (d1char, stdout);

	    SIGNAL_ACTION(SIGUSR1, oncld, NULL);	/* restore handler */

	    set_io (MUMPS);
	    if (ierr == STORE) {
		zsystem = 1;
		goto err;
	    }
	    goto skip_line;
	}
	ierr = CMMND;
	goto err;
    }

    
    mcmnd = ch | 0140;			/* uppercase to lower case */
    i = 1;
    while ((ch = (*codptr)) != SP && ch != ':' && ch != EOL) {
	tmp3[++i] = ch | 0140;
	codptr++;
    }
    j = i;

    if (j > 1) {
	tmp3[0] = SP;
	tmp3[1] = mcmnd;
	tmp3[++j] = SP;
	tmp3[++j] = EOL;
	if (mcmnd != 'z') {
	    if (find (
			" ab ablock assign asta astart asto astop aunb aunblock \
 break close do else esta estart esto estop etr etrigger for goto hang halt \
 if job kill ks ksubscripts kv kvalue lock merge new open quit read rl rload\
 rs rsave set tc tcommit th then tre trestart tro trollback ts tstart use\
 view write xecute "
			 ,tmp3) == FALSE) {
		ierr = CMMND;
		goto err;
	    }

	    switch (mcmnd) {
	      case 'a':	
		if      (tmp3[2]=='b') mcmnd=ABLOCK;
		else if (tmp3[2]=='u') mcmnd=AUNBLOCK;
		else if (tmp3[4]=='a') mcmnd=ASTART;
		else if (tmp3[4]=='o') mcmnd=ASTOP;
		break;
	     case 'e':
		if      (tmp3[2]=='t') mcmnd=ETRIGGER;
		else if (tmp3[4]=='a') mcmnd=ESTART;
		else if (tmp3[4]=='o') mcmnd=ESTOP;
		break;
	     case 'h':
	        mcmnd = tmp3[4]=='t' ? HALT : HANG;
		break;
             case 'k':
		if      (tmp3[2] == 'v') mcmnd=KVALUE;
	        else if (tmp3[2] == 's') mcmnd=KSUBSC;
		break;
	     case 'r':
		if      (tmp3[2] == 'l') mcmnd=RLOAD;
	        else if (tmp3[2] == 's') mcmnd=RSAVE;
		break;
	     case 't':
		if      (tmp3[2] == 'c') mcmnd = TCOMMIT;
		else if (tmp3[2] == 'h') mcmnd = THEN;
		else if (tmp3[3] == 'e') mcmnd = TRESTART;
		else if (tmp3[3] == 'o') mcmnd = TROLLBACK;
	       } /* end of switch(mcmnd) */
	} else {
	    if (standard) {
		ierr = NOSTAND;
		goto err;
	    }
	    mcmnd = tmp3[2] - 32;	/* z_command select */
	    if (find (zcommds, tmp3) == FALSE)
		mcmnd = PRIVATE;
	}
    }
    if (*codptr == ':') {		/* handle postconditional */
/* postcond after FOR,IF,ELSE not allowed */ if (mcmnd == FOR || mcmnd == IF || mcmnd == ELSE) {
	    ierr = SPACER;
	    goto err;
	}
	codptr++;
	expr (STRING);
	if (ierr > OK)
	    goto err;
	ch = *codptr;
	if (ch != SP && ch != EOL) {
	    ierr = SPACER;
	    goto err;
	}
	if (tvexpr (argptr) == FALSE) {	/* skip arguments */
	    mcmnd = 0;			/* avoid false LEVEL error */
	    for (;;)
	    {
		if (ch == EOL)
		    goto skip_line;
		if ((ch = *++codptr) == SP)
		    goto next_cmnd;
		if (ch != '"')
		    continue;
		while (*codptr++ != EOL) {
		    if (*codptr != ch)
			continue;
		    if (*++codptr != ch)
			break;
		}
		if (--codptr == code)
		    goto err;
	    }
	}
    }
    if (*codptr != EOL) {		/* beware argumentless cmnds at end of line */
	codptr++;			/* entry for next argument in list */
      again:while (*codptr == '@') {	/* handle indirection */
	    stcpy (tmp, codptr++);	/* save code to restore on nameind */
	    expr (ARGIND);
	    if (ierr > OK)
		goto err;
	    if (((ch = (*codptr)) != SP && ch != EOL &&
		 ch != ',' && ch != ':' && ch != '=') ||
		(ch == '@' && *(codptr + 1) == '(')) {
		stcpy (code, tmp);	/* restore code on nameind */
		codptr = code;
		break;
	    } else {
		stcpy (argptr + stlen (argptr), codptr);
		stcpy (code, argptr);
		codptr = code;
	    }
	}
    }
    switch (mcmnd) {
    case SET:

      set0:
	if ((ch = (*codptr)) >= 'A') {	/* no set$piece nor multiset */
	    if (*(codptr + 1) == '=' &&	/* single char local variable */
		*(codptr + 2) != '=') {
		if ((ch > 'Z' && ch < 'a') || ch > 'z') {
		    ierr = INVREF;
		    break;
		}
		vn[0] = ch;
		vn[1] = EOL;
		codptr += 2;
		expr (STRING);
		if (ierr > OK)
		    break;
		symtab (set_sym, vn, argptr);
		goto set1;
	    }
	    expr (NAME);
	    if (ierr > OK)
		break;
	    stcpy (vn, varnam);
	    if ((*++codptr != '=') ||
		(*(codptr + 1) == '=')) {
		ch = *codptr;
/* double char symbol ** (power) is encoded by ' ' */
		if (ch == '*' && *(codptr + 1) == ch) {
		    codptr++;
		    ch = ' ';
		}
/* negated boolean operator */
		else if ((ch == '\'') && (*(codptr + 2) == '='))
		    ch = SETBIT (*++codptr);
		if (*++codptr != '=') {
/* SET A++ or SET A-- equivalent to SET A+=1 SET A-=1 currently disabled */
#ifdef NEVER
		    if ((ch == '+' || ch == '-') && ch == *codptr) {
			codptr++;
			setop = ch;
			argptr[0] = '1';
			argptr[1] = EOL;
			goto set2;
		    }
#endif /* NEVER */
		    ierr = ASSIGNER;
		    break;
		}
		setop = ch;
	    }
	    codptr++;
	    expr (STRING);
	    if (ierr > OK)
		break;
	  set2:if (vn[0] == '^')
		  global  (set_sym, vn, argptr);
	       else
	  	  symtab (set_sym, vn, argptr);
	       if (ierr > OK) {
	  	  stcpy (varerr, vn);
		  break;
	       }
	  set1:if (*codptr != ',')
		break;
	    if (*++codptr == '@')
		goto again;
	    goto set0;
	}
/****** special SET syntax: multiple SET, set$piece, special variables */
	{
	    char    multiset,
	            vnset[256];		/* multiset variables */
	    long    arg3,
	            arg4;		/* 3rd,4th arg in set$piece */

	    if ((multiset = (ch == '('))) {
		vnset[0] = EOL;
		codptr++;
	    }
	set:
	    if (*codptr == '$' && (*(codptr + 1) | 0140) == 'p') {	/* set$piece */
		if (multiset) {
		    ierr = INVREF;
		    goto err;
		}
		setpiece = 'p';
		while (*++codptr != '(') {
		    if (*codptr == EOL) {
			ierr = INVREF;
			goto err;
		    }
		}
		codptr++;
		expr (NAME);
		if (ierr > OK)
		    goto err;
		stcpy (vn, varnam);
		codptr++;
		if (*codptr++ != ',') {
		    ierr = COMMAER;
		    goto err;
		}
		expr (STRING);
		if (ierr > OK)
		    goto err;
		stcpy (tmp2, argptr);
		if (*codptr != ')') {
		    codptr++;
		    expr (STRING);
		    if (ierr > OK)
			goto err;
		    arg3 = intexpr (argptr);
		    if (ierr == MXNUM) {
			arg3 = 256;
			ierr = OK;
		    }
		} else
		    arg3 = 1;
		if (*codptr != ')') {
		    codptr++;
		    expr (STRING);
		    if (ierr > OK)
			goto err;
		    if (*codptr != ')') {
			ierr = BRAER;
			goto err;
		    }
		    arg4 = intexpr (argptr);
		    if (ierr == MXNUM) {
			arg4 = 256;
			ierr = OK;
		    }
		} else
		    arg4 = arg3;
	    } else if (*codptr=='$' && (*(codptr+1)|0140)=='q'  
                                    && (*(codptr+2)|0140)=='s') {
	        /*SET $QSUBSCRIPT*/
                if (multiset) { 
		    ierr=INVREF; 
		    goto err; 
	        }
                setpiece='q';
                while(*++codptr!='(') {
                  if (*codptr==EOL) {
                     ierr=INVREF; 
		     goto err;
                  }
                }
                codptr++; 
                expr(NAME); 
	        if (ierr>OK) 
		   goto err;
                stcpy(vn,varnam);
                if (*++codptr==',') {
              	    codptr++; 
              	    expr(STRING); 
		    if (ierr>OK) 
		        goto err;
	            stcpy(tmp2,argptr);
                }
                if (*codptr!=')') { 
		    ierr=BRAER; 
		    goto err; 
	        }
	    } else if (*codptr == '$' && (*(codptr + 1) | 0140) == 'e') {	
		/* set $extract */
		if (multiset) {
		    ierr = INVREF;
		    goto err;
		}
		setpiece = 'e';
		while (*++codptr != '(') {
		    if (*codptr == EOL) {
			ierr = INVREF;
			goto err;
		    }
		}
		codptr++;
		expr (NAME);
		if (ierr > OK)
		    goto err;
		stcpy (vn, varnam);
		codptr++;
		if (*codptr != ')') {
		    codptr++;
		    expr (STRING);
		    if (ierr > OK)
			goto err;
		    arg3 = intexpr (argptr);
		    if (ierr == MXNUM) {
			arg3 = 256;
			ierr = OK;
		    }
		} else
		    arg3 = 1;
		if (*codptr != ')') {
		    codptr++;
		    expr (STRING);
		    if (ierr > OK)
			goto err;
		    if (*codptr != ')') {
			ierr = BRAER;
			goto err;
		    }
		    arg4 = intexpr (argptr);
		    if (ierr == MXNUM) {
			arg4 = 256;
			ierr = OK;
		    }
		} else
		    arg4 = arg3;
	    } else if (*codptr == '$' && (*(codptr + 1) | 0140) == 'g' && cset) {	/* set$get */
		if (multiset) {
		    ierr = INVREF;
		    goto err;
		}
		setpiece = 'g';
		while (*++codptr != '(') {
		    if (*codptr == EOL) {
			ierr = INVREF;
			goto err;
		    }
		}
		codptr++;
		expr (NAME);
		if (ierr > OK)
		    goto err;
		stcpy (vn, varnam);
		if (*++codptr == ',') {
		    codptr++;
		    expr (STRING);
		    if (ierr > OK)
			goto err;
		    stcpy (tmp2, argptr);
		} else
		    tmp2[0] = EOL;
		if (*codptr != ')') {
		    ierr = BRAER;
		    goto err;
		}
	    } else {
		if (*codptr == '$') {
		    codptr++;
		    expr (NAME);
		    if (ierr > OK)
			goto err;
		    stcpy (tmp, varnam);
		    varnam[0] = '$';
		    stcpy (&varnam[1], tmp);
		    i = 0;
		    while ((ch = varnam[++i]) != EOL)
			if (ch >= 'A' && ch <= 'Z')
			    varnam[i] |= 0140;	/*to lowercase */
		} else {
		    expr (NAME);
		    if (ierr > OK)
			goto err;
		}
		stcpy (vn, varnam);
	    }
	    if (multiset) {
		vnset[i = stlen (vnset)] = SOH;
		stcpy (&vnset[++i], vn);
		if (*++codptr == ',') {
		    codptr++;
		    goto set;
		}
		if (*codptr != ')') {
		    ierr = COMMAER;
		    goto err;
		}
	    }
	    if (*++codptr != '=') {
		ch = *codptr;
		if (!cset || *++codptr != '=' ||
		    multiset || setpiece || varnam[0] == '$') {
		    ierr = ASSIGNER;
		    break;
		}
		setop = ch;
	    }
	    codptr++;
	    expr (STRING);
	    if (ierr > OK)
		goto err;

	    if (multiset)
	      multi:
	    {
		i = 0;
		while (vnset[i] == SOH)
		    i++;
		j = 0;
		while ((vn[j] = vnset[i]) != SOH && vnset[i] != EOL) {
		    vnset[i++] = SOH;
		    j++;
		}
		vn[j] = EOL;
		if (j == 0)
		    goto s_end;
	    }
	    if (setpiece == 'p') {
		short   m,
		        n;

		if (arg4 < arg3 || arg4 < 1) {
		    setpiece = FALSE;
		    break;
		}
		if (arg3 <= 0)
		    arg3 = 1;
		if (vn[0] == '^')
		    global  (get_sym, vn, tmp3);

		else
		    symtab (get_sym, vn, tmp3);
		if (ierr == UNDEF || ierr == (UNDEF - CTRLB)) {
		    tmp3[0] = EOL;
		    ierr = OK;
		} else if (ierr != OK)
		    stcpy (varerr, vn);

		ch = 0;
		m = 0;
		n = 0;
		j = stlen (tmp2);
		while (n < arg3 - 1) {
		    if ((ch = find (&tmp3[m], tmp2)) <= 0) {
			while (++n < arg3)
			    if (stcat (tmp3, tmp2) == 0) {
				ierr = MXSTR;
				goto err;
			    }
			arg3 = arg4 = stlen (tmp3);
			goto set10;
		    }
		    n++;
		    m += j + ch - 1;
		}
		if (arg3 > 1)
		    arg3 = m;
		else
		    arg3 = 0;
		while (n++ < arg4) {
		    if ((ch = find (&tmp3[m], tmp2)) <= 0) {
			arg4 = stlen (tmp3);
			goto set10;
		    }
		    m += j + ch - 1;
		}
		arg4 = m - j;
	      set10:;
		stcpy0 (tmp2, tmp3, (long) arg3);
		tmp2[arg3] = EOL;
		if (stcat (tmp2, argptr) == 0) {
		    ierr = MXSTR;
		    goto err;
		}
		if (stcat (tmp2, &tmp3[arg4]) == 0) {
		    ierr = MXSTR;
		    goto err;
		}
		stcpy (argptr, tmp2);
		setpiece = FALSE;
	    } else if (setpiece=='q') { /* SET$QSUBSCRIPT */
                setpiece=FALSE;
	        if (vn[0]=='^') global(get_sym,vn,tmp3);
	        else            symtab(get_sym,vn,tmp3);
	        if (ierr==UNDEF || ierr==(UNDEF-CTRLB)) {
	            tmp3[0]=EOL; ierr=OK;
	        }
	        else if (ierr!=OK) stcpy(varerr,vn);
	        if (ierr==OK) {
	           /* 2nd $QS argument */
	           if ((arg4=intexpr(tmp2))<-1) ierr=ARGER;
                   if (ierr!=OK) break;

                   /* special if source is empty */
                   if (tmp3[0]!=EOL || (arg4!=0)) {

                   /* special: Set env to empty: no |""| */
                   if ((arg4==-1) && (*argptr==EOL)) tmp2[0]=EOL;
                   /* put replacement string in tmp2 with     */
                   /* quotes around env or subscript, unless numeric*/
                   else if ((arg4!=0) && !znamenumeric(argptr)) {
                         i=0; j=-1;
                         tmp2[0]='"';
                         while((tmp2[++i]=argptr[++j]) != EOL) {
                           if (tmp2[i]=='"') tmp2[++i]='"';
                           if (i>=(STRLEN-2)) { ierr=MXSTR; break; }
                        }
                   tmp2[i]='"'; tmp2[++i]=EOL;
                   }
                   else stcpy(tmp2,argptr);

                   /* source is tmp3, dest is argptr, replacement is tmp2 */
                   { int ch,cpflag,quote,piececounter;
                       piececounter=0;
                       i=0; 
		       j=0;
                       quote=FALSE;
                       cpflag=FALSE;
                       /* if source has no env, process right now */
                       if ((arg4==-1) &&
                           (tmp3[tmp3[0]=='^']!='|') &&
                            tmp2[0]!=EOL) {
                           if (tmp3[0]=='^') { argptr[j++]='^'; i=1; }
                           argptr[j++]='|';
                           ch=0; 
			   while ((argptr[j]=tmp2[ch++])!=EOL) j++;
                           argptr[j++]='|';
                       }
	               else if (arg4==0) {/* '^'+name may be separated by env */
                           if (tmp2[0]=='^') argptr[j++]='^';
                           if (tmp3[0]=='^') i++;
                       }
                       while ((ch=tmp3[i++])!=EOL) {
                           if (ch=='"') quote= !quote;
                           if (!quote) {
                               if (ch==',') {
                                   piececounter++; 
				   argptr[j++]=ch;
				   continue;
				}   
                               if ((ch=='(' && piececounter==0)) {
                                   if (!cpflag && (arg4==0)) i--;
                                   else {
                                      	piececounter=1; 
					argptr[j++]=ch;
					continue;
	                           }
                               }
                               if (ch=='|') {
                                  if (piececounter==0)
				      piececounter=(-1);
                                  else if (piececounter==(-1))
                                      piececounter=0;
                                  if (tmp2[0]!=EOL || piececounter>0)
                                      argptr[j++]=ch;
                                  continue;
                               }
                          }
                          if (piececounter==arg4) {
                             if (cpflag) continue; cpflag=TRUE;
                             ch=0; if (arg4==0 && tmp2[0]=='^') ch=1;
                             while ((argptr[j]=tmp2[ch++])!=EOL) j++;
                          }
                          else argptr[j++]=ch;
                          if (j>=(STRLEN-1)) { ierr=MXSTR; break; }
                      }
                      if (piececounter && (piececounter==arg4))argptr[j++]=')';
                      if (piececounter<arg4) {
                          if (piececounter==0) argptr[j++]='(';
                          else                 argptr[j-1]=',';
                          while (++piececounter<arg4) {
                            argptr[j++]='"'; 
                            argptr[j++]='"'; 
                            argptr[j++]=',';
                            if (j>=STRLEN) { ierr=MXSTR; break; }
                          }
                      }
                      ch=0; 
                      while ((argptr[j++]=tmp2[ch++])!=EOL);
                      argptr[j-1]=')';
                   }
                   argptr[j]=EOL;
                   if (j>=STRLEN) { ierr=MXSTR; break; }
                }
              }
              else break;            

	    } else if (setpiece == 'e') {	/* SETtable $EXTRACT *//* parameters ok?? */
		if (arg3 > arg4 || arg4 < 1) {
		    setpiece = FALSE;
		    break;
		}
		if (arg3 <= 0)
		    arg3 = 1;
		if (arg3 > STRLEN) {
		    ierr = MXSTR;
		    goto err;
		}
/* get value of glvn */
		if (vn[0] == '^')
		    global  (get_sym, vn, tmp3);

		else
		    symtab (get_sym, vn, tmp3);
/* if UNDEF assume null string */
		if (ierr == UNDEF || ierr == (UNDEF - CTRLB)) {
		    tmp3[0] = EOL;
		    ierr = OK;
		} else if (ierr != OK)
		    stcpy (varerr, vn);

		j = stlen (tmp3);
/* pad with SPaces if source string is too short */
		while (j < arg3)
		    tmp3[j++] = SP;
		tmp3[j] = EOL;
		if (stlen (tmp3) > arg4)
		    stcpy (tmp2, &tmp3[arg4]);
		else
		    tmp2[0] = EOL;
		tmp3[arg3 - 1] = EOL;
/* compose new value of glvn */
		if (stcat (tmp3, argptr) == 0) {
		    ierr = MXSTR;
		    goto err;
		}
		if (stcat (tmp3, tmp2) == 0) {
		    ierr = MXSTR;
		    goto err;
		}
		stcpy (argptr, tmp3);
		setpiece = FALSE;
	    } else if (setpiece == 'g') {	/* SETtable $GET */
		setpiece = FALSE;
		ch = (stcmp (tmp2, argptr) == 0) ? killone : set_sym;
		if (vn[0] == '^')
		    global  (ch, vn, argptr);

		else
		    symtab (ch, vn, argptr);
		if (ierr != OK)
		    stcpy (varerr, vn);
		break;
	    }
	    if (vn[0] == '^') {		/* global variables */
		global  (set_sym, vn, argptr);

		if (ierr > OK) {
		    stcpy (varerr, vn);
		    goto err;
		}
	    } else if (vn[0] != '$') {	/* local variables */
		symtab (set_sym, vn, argptr);
		if (ierr > OK) {
		    stcpy (varerr, vn);
		    goto err;
		}
	    } else {			/* $-variables */
		if (vn[1] == 'x') {	/* set $X */
		    j = intexpr (argptr);
		    if (ierr == MXNUM) {
			j = 256;
			ierr = OK;
		    }
		    if (j < 0)
			j = 0;
		    if (io == HOME) {
			argptr[0] = ESC;
			argptr[1] = '[';
			argptr[2] = EOL;
			if (ypos[HOME] > 1) {
			    intstr (tmp3, ypos[HOME] + 1);
			    stcat (argptr, tmp3);
			}
			if (j > 0) {
			    stcat (argptr, ";\201");
			    intstr (tmp3, j + 1);
			    stcat (argptr, tmp3);
			}
			stcat (argptr, "H\201");
			write_m (argptr);
		    }
		    xpos[io] = j;
		    goto s_end;
		} else if (vn[1] == 'y') {	/* set $Y */
		    j = intexpr (argptr);
		    if (ierr == MXNUM) {
			j = 256;
			ierr = OK;
		    }
		    if (j < 0)
			j = 0;
		    if (io == HOME) {
			argptr[0] = ESC;
			argptr[1] = '[';
			argptr[2] = EOL;
			if (j > 0) {
			    intstr (tmp3, j + 1);
			    stcat (argptr, tmp3);
			}
			if (xpos[HOME] > 0) {
			    stcat (argptr, ";\201");
			    intstr (tmp3, xpos[HOME] + 1);
			    stcat (argptr, tmp3);
			}
			stcat (argptr, "H\201");
			write_m (argptr);
		    }
		    ypos[io] = j;
		    goto s_end;
		} else if (vn[1] == 't') {	/* set $t */
		    test = tvexpr (argptr);
		    goto s_end;
		} else if (vn[1] == 'j') {	/* set $job */
		    pid = intexpr (argptr);
		    lock (" \201", -1, 's');	/* tell lock about SET */
		    goto s_end;
		} else if (vn[1] == 'h') {	/* set $horolog */
		    long int day,
		            sec;

		    sec = 0L;
		    for (i = 0; argptr[i] != EOL; i++) {
			if (argptr[i] == ',') {
			    sec = intexpr (&argptr[i + 1]);
			    break;
			}
		    }
		    if (sec < 0 || sec >= 86400L) {
			ierr = ARGER;
			goto err;
		    }
		    day = intexpr (argptr) - 47117L;
		    if (day < 0 || day > 49710L) {
			ierr = ARGER;
			goto err;
		    }
		    sec += day * 86400L + FreeM_timezone;
		    day = FreeM_timezone;
#if defined (__CYGWIN__) || defined (FREEBSD)
                    ierr = PROTECT;
                    goto err;
#else
  		    if (stime (&sec)) {
			ierr = PROTECT;
			goto err;

		    }

#ifndef LINUX
/* daylight savings time status may have changed */
		    {
			struct tm *ctdata;
			long    clock;

			clock = time (0L);
			ctdata = localtime (&clock);
			if (day -= (FreeM_timezone = ctdata->tm_tzadj)) {
			    sec -= day;
			    tzoffset += day;
			    stime (&sec);
			}
		    }
#endif /* LINUX */
#endif /* __CYGWIN__ or FREEBSD */
		    goto s_end;
		} else if ((vn[1] == 'r') ||	/* set $reference */
		  ((vn[1] == 'z') && (vn[2] == 'r') && vn[3] == EOL)) {
		    if (argptr[0] == EOL) {
			zref[0] = EOL;
			break;
		    }
		    stcpy (tmp4, codptr);
		    stcpy (code, argptr);
		    codptr = code;
		    expr (NAME);
		    stcpy (code, tmp4);
		    codptr = code;
		    if (argptr[0] != '^')
			ierr = INVREF;
		    if (ierr <= OK)
			nakoffs = stcpy (zref, argptr);		/* save reference */
		    goto s_end;
		} else if (vn[1] == 'z') {	/* $Z.. variables *//* if not intrinsic: make it user defined */
		    i = stcpy (&tmp[1], &vn[1]) + 1;
		    if (vn[3] == DELIM)
			i = 3;		/* set $zf() function keys */
		    tmp[0] = SP;
		    tmp[i] = SP;
		    tmp[++i] = EOL;
		    if (find (zsvn, tmp) == FALSE) {
			i = 2;
			while (vn[i] != EOL)
			    if (vn[i++] == DELIM) {
				ierr = INVREF;
				goto err;
			    }
			udfsvn (set_sym, &vn[2], argptr);
			break;
		    }
		    if ((!stcmp (&vn[2], "l\201")) ||	/* set $zlocal */
			(!stcmp (&vn[2], "local\201"))) {
			if (argptr[0] == EOL) {
			    zloc[0] = EOL;
			    break;
			}
			stcpy (tmp4, codptr);
			stcpy (code, argptr);
			codptr = code;
			expr (NAME);
			stcpy (code, tmp4);
			codptr = code;
			if (argptr[0] == '^')
			    ierr = INVREF;
			if (ierr <= OK)
			    stcpy (zloc, argptr);	/* save reference */
			break;
		    }
		    if ((!stcmp (&vn[2], "t\201")) ||
			(!stcmp (&vn[2], "tr\201")) ||
			(!stcmp (&vn[2], "trap\201"))) {	/* set $ztrap */
			if (stlen (argptr) > ZTLEN) {
			    ierr = MXSTR;
			    goto err;
			}
/* DSM V.2 error trapping */
#ifdef DEBUG_NEWSTACK
                        printf("Setting Ztrap, DSM2err [%d]\r\n",DSM2err);
#endif
#ifdef NEWSTACK
                      if(!stack) {
                         tmp_stack = NEW(stack_level,1);
                         printf("Stack goes up...\r\n");
	                 if (!tmp_stack) { ierr = STKOV; goto err; } 
                         tmp_stack->previous = stack; stack = tmp_stack;
#ifdef DEBUG_NEWSTACK
			if(!cmdptr) printf("cmdptr is zero!\r\n");
#endif
                         tmp_stack->cmdptr = cmdptr;
                      } else { tmp_stack = stack; }
                      stcpy(tmp_stack->ztrap, argptr);
#else
			if (DSM2err) 
			    stcpy (ztrap[NESTLEVLS + 1], argptr);
			else
			    stcpy (ztrap[nstx], argptr);
#endif
		    } else if (!stcmp (&vn[2], "p\201") ||	/* set $zprecision */
			       !stcmp (&vn[2], "precision\201")) {
			if ((zprecise = intexpr (argptr)) < 0)
			    zprecise = 0;
			if (ierr == MXNUM) {
			    zprecise = 99;
			    ierr = OK;
			}
			if (zprecise > 99)
			    zprecise = 99;
		    } else if (vn[2] == 'f' && vn[3] == DELIM) {	/* set $zf() function keys */
			i = intexpr (&vn[4]) - 1;
			if (i < 0 || i > 43) {
			    ierr = FUNARG;
			    goto err;
			}
			if (stlen (argptr) > FUNLEN) {
			    ierr = MXSTR;
			    goto err;
			}
			stcpy (zfunkey[i], argptr);
		    }
/* set $zm_ loadable match */
		    else if (vn[2] == 'm' && vn[4] == EOL &&
			(vn[3] == 'c' || vn[3] == 'n' || vn[3] == 'p' ||
			 vn[3] == 'l' || vn[3] == 'u')) {	/* sort match code */
			short   k;

			i = 0;
			for (ch = 0; ch <= 255; ch++) {
			    j = argptr - partition;
			    while ((k = partition[j++]) != EOL) {
				if (UNSIGN (k) == ch) {
				    tmp[i++] = k;
				    break;
				}
			    }
			}
			tmp[i] = EOL;
			switch (vn[3]) {
			case 'c':
			    stcpy (zmc, tmp);
			    break;
			case 'n':
			    stcpy (zmn, tmp);
			    break;
			case 'p':
			    stcpy (zmp, tmp);
			    break;
/*   'a': always union of zml+zmu */
			case 'l':
			    stcpy (zml, tmp);
			    break;
			case 'u':
			    stcpy (zmu, tmp);
			    break;
/*   'e': always 'everything'     */
			}

		    } else {
			ierr = INVREF;
			break;
		    }
		} else {
		    ierr = INVREF;
		    goto err;
		}
	    } /* svns=$vars */

	    if (multiset)
		goto multi;
	}
      s_end:if (*codptr != ',')
	    break;
	if (*++codptr == '@')
	    goto again;
	goto set0;

    case IF:
	if (*codptr == SP || *codptr == EOL) {	/* no argument form of IF */
	    if (test)
		goto next_cmnd;
	    goto skip_line;
	}
	expr (STRING);
	if (*argptr == '1') {
	    test = TRUE;
	    break;
	}
	if (ierr > OK)
	    break;
	if (*argptr == '0' && argptr[1] == EOL) {
	    test = FALSE;
	    goto skip_line;
	}
	if ((test = tvexpr (argptr)) == FALSE)
	    goto skip_line;
	break;


    case WRITE:

	if (io != HOME && devopen[io] == 'r') {
	    ierr = NOWRITE;
	    goto err;
	}
	if ((i = (*codptr)) == SP || i == EOL) {
	    if (standard) {
		ierr = NOSTAND;
		break;
	    }
	    goto zwrite;
	}
      writeproc:
	switch (i) {
	case '!':
	    if (crlf[io])
		write_m ("\012\201");
	    else
		write_m ("\015\012\201");
	    break;
	case '#':
	    write_m ("\015\014\201");
	    break;
	case '?':
	    codptr++;
	    expr (STRING);
	    if (ierr > OK)
		goto err;
	    write_t ((short) intexpr (argptr));
	    break;
	case '/':
	    codptr++;
	    expr (NAME);
	    if (ierr > OK)
		goto err;
	    write_f (varnam);
	    codptr++;
	    break;
	case '*':
	    codptr++;
	    expr (STRING);
	    if (ierr > OK)
		goto err;
	    argptr[0] = (char) UNSIGN (intexpr (argptr));
	    argptr[1] = EOL;
/* special treatment for EOL as long as we don't have 8 bit */
	    if (argptr[0] == EOL) {
		mcmnd = '*';
		m_output (argptr);
		mcmnd = WRITE;
	    } else
		write_m (argptr);
	    break;
	default:
	    expr (STRING);
	    if (ierr > OK)
		goto err;
	    write_m (argptr);
	}
	if ((i == '!') || (i == '#')) {
	    if ((i = *++codptr) == '!' ||
		i == '#' ||
		i == '?')
		goto writeproc;
	}
	break;

    case READ:

	if (io != HOME && devopen[io] != 'r' && devopen[io] != '+') {
	    ierr = NOREAD;
	    goto err;
	}
      read_command:

	switch (*codptr) {
	case '!':
	    if (crlf[io])
		write_m ("\012\201");
	    else
		write_m ("\012\015\201");
	    if (*++codptr == '!' ||
		*codptr == '#' ||
		*codptr == '?')
		goto read_command;
	    goto cont_read;
	case '#':
	    write_m ("\015\014\201");
	    if (*++codptr == '!' ||
		*codptr == '#' ||
		*codptr == '?')
		goto read_command;
	    goto cont_read;
	case '?':
	    codptr++;
	    expr (STRING);
	    if (ierr > OK)
		goto err;
	    write_t ((short) intexpr (argptr));
	    goto cont_read;
	case '/':
	    codptr++;
	    expr (NAME);
	    if (ierr > OK)
		goto err;
	    write_f (varnam);
	    codptr++;
	    goto cont_read;
	case '"':
	    i = 0;
	    for (;;)
	    {
		while ((ch = *++codptr) > '"')
		    argptr[i++] = ch;
/* EOL < "any ASCII character" */
		if (ch == '"' && (ch = *++codptr) != '"') {
		    argptr[i] = EOL;
		    write_m (argptr);
		    goto cont_read;
		}
		if ((argptr[i++] = ch) == EOL) {
		    ierr = QUOTER;
		    goto err;
		}
	    }
	}

	i = InFieldLen;			/* no length limit */
	InFieldLen = 255;		/* Not necessarily tied to STRLEN*/
	if (*codptr == '*') {
	    codptr++;
	    i = 0;
	}				/* single char read */
	if (*codptr == '$') {
	    ierr = INVREF;
	    goto err;
	}
	expr (NAME);
	if (ierr > OK)
	    goto err;
	stcpy (vn, varnam);
	codptr++;			/* lvn */

	if (i != 0 && *codptr == '#') {	/* length limit */
	    codptr++;
	    expr (STRING);
	    if ((i = intexpr (argptr)) <= 0)
		ierr = ARGER;
	    if (ierr > OK)
		goto err;
	}
	timeout = (-1L);
	timeoutms = 0;			/* no timeout */
	if (*codptr == ':') {		/* timeout */
	    int     i,
	            ch;

	    codptr++;
	    expr (STRING);
	    numlit (argptr);
	    if (ierr > OK)
		goto err;
	    timeout = 0;
	    timeoutms = 0;
	    if (argptr[0] != '-') {
		i = 0;
		for (;;)		/* get integer and fractional part */
		{
		    if ((ch = argptr[i++]) == EOL)
			break;
		    if (ch == '.') {
			timeoutms = (argptr[i++] - '0') * 100;
			if ((ch = argptr[i++]) != EOL) {
			    timeoutms += (ch - '0') * 10;
			    if ((ch = argptr[i]) != EOL) {
				timeoutms += (ch - '0');
			    }
			}
			break;
		    }
		    timeout = timeout * 10 + ch - '0';
		}
	    }
	}
	read_m (argptr, timeout, timeoutms, i);
	if (vn[0] != '^')
	    symtab (set_sym, vn, argptr);
	else
	    global  (set_sym, vn, argptr);

	if (ierr != OK)
	    stcpy (varerr, vn);
      cont_read:
	break;

    case ELSE:
	if (*codptr != EOL) {
	    if (*codptr != SP) {
		ierr = ARGER;
		break;
	    }
	    if (test == FALSE)
		goto next_cmnd;		/* same line */
	}
	goto skip_line;			/* next line */

    case QUIT:
#ifdef DEBUG_NEWSTACK
       printf("At QUIT command, checking stack...\r\n");
#endif
#ifdef NEWSTACK
#ifdef DEBUG_NEWSTACK
       printf("codptr is [%d] (vs EOL (%d) or SP (%d)\r\n",*codptr,EOL,SP);
       printf("stack is (%d), stack->command is (%d)\r\n",stack,
              (stack) ? stack->command : 0);
#endif
    if (*codptr != EOL && *codptr != SP && stack && stack->command != '$') {
#else
#ifdef DEBUG_NEWSTACK
       printf("nestc[nstx] is (%d)\r\n",nestc[nstx]);
#endif
	if (*codptr != EOL && *codptr != SP && nestc[nstx] != '$') {
#endif
#ifdef DEBUG_NEWSTACK
       printf("IERR\r\n");
#endif
	    ierr = ARGER;
	    break;
	}
#ifdef NEWSTACK
        if (stack && stack->command == '$') {
#else
	if (nestc[nstx] == '$') {	/* extrinsic function/variable */
#endif
#ifdef DEBUG_NEWSTACK
       printf("EXTRINSIC\r\n");
#endif
	    if (*codptr == EOL || *codptr == SP) {
#ifdef DEBUG_NEWSTACK
       printf("CODPTR is [%d]\r\n",*codptr);
#endif
		if (exfdefault[0] == EOL) {
		    *argptr = EOL;
		    ierr = NOVAL;
		}
/* there is a default expression... */
		else {
		    stcpy (&code[1], exfdefault);
		    expr (STRING);
		    if (ierr != OK - CTRLB && ierr != OK && ierr != INRPT) {
#ifdef DEBUG_NEWSTACK
       printf("Break at 1st IERR\r\n");
#endif
			break;
                    }
		}
	    } else {
		expr (STRING);
		if (ierr != OK - CTRLB && ierr != OK && ierr != INRPT) {
#ifdef DEBUG_NEWSTACK
       printf("Break at 2nd IERR\r\n");
#endif
		    break;
                }
	    }
#ifdef DEBUG_NEWSTACK
            printf("CHECK 01 (Stack POP)\r\n"); 
#endif
#ifdef NEWSTACK
            /* Pop off the stack */
            if(stack) { 
              if(stack->new) unnew();
              if(stack->rouname) {
                stcpy (rou_name, stack->rouname); zload (rou_name);
                dosave[0] = 0;
              } 
              level  = stack->level;
              roucur = stack->roucur + rouptr;
              stcpy(codptr = code, cmdptr = stack->cmdptr);
              tmp_stack = stack; stack = stack->previous;
              free(tmp_stack);
              if(stack) forsw = (stack->command == FOR); else forsw = 0;
              loadsw = TRUE; return 0;
            }
#else /* NEWSTACK */
	    if (nestn[nstx]) {		/* reload routine */
		namptr = nestn[nstx];
		stcpy (rou_name, namptr);
		zload (rou_name);
		dosave[0] = 0;
		namptr--;
	    }
	    if (nestnew[nstx])
		unnew ();		/* un-NEW variables */
/* restore old pointers */
	    level = nestlt[nstx];	/* pop level */
	    roucur = nestr[nstx] + rouptr;
	    stcpy (codptr = code, cmdptr = nestp[nstx--]);
	    forsw = (nestc[nstx] == FOR);
	    loadsw = TRUE;
	    return 0;
#endif /* NEWSTACK */
	}
#ifdef NEWSTACK
        if(stack && stack->command == BREAK) { ierr = OK - CTRLB; goto zgo; }
#else
	if (nestc[nstx] == BREAK) {
	    ierr = OK - CTRLB;
	    goto zgo;
	}				/*cont. single step */
#endif
      quit0:
#ifdef DEBUG_NEWSTACK
        printf("CHECK 02 (Stack POP)\r\n"); 
#endif
#ifdef NEWSTACK
        /* Pop off the stack */
        if(stack && !stack->cmdptr) { 
          tmp_stack = stack; stack = stack->previous; free(tmp_stack);
        }
        if(!stack) goto restore;
        if(stack) { 
          if(stack->new) unnew();
          if(stack->command == FOR) {
            stcpy(code, cmdptr = stack->cmdptr);
            codptr = code;
	    ftyp = fortyp[--forx]; fvar = forvar[forx]; finc = forinc[forx];
	    flim = forlim[forx]; fi = fori[forx];
	    if ((forsw = (stack->command == FOR))) goto for_end;
	    goto next_line;
          }
          if(stack->rouname) {
            namptr = stack->rouname;
            if((stack->command != XECUTE) || loadsw) {
              stcpy (rou_name, namptr); zload (rou_name);
              dosave[0] = 0;
            }
          } 
	  if ((mcmnd = stack->command) == BREAK) goto restore;		
	  if (mcmnd == DO_BLOCK) { test = stack->level; level--; }
	  else level = stack->level;
	  roucur = stack->roucur + rouptr;
#ifdef DEBUG_NEWSTACK
          printf("Stack (%d), stack->cmdptr (%d)\r\n", stack, stack->cmdptr);
          if(!stack->cmdptr) { 
            printf("Unset CMDPTR!\r\n");
            printf("Level (%d), Prev (%d), Command (%d-%c), Roucur (%d)\r\n",
                   stack->level, stack->previous, stack->command, 
                   stack->command, stack->roucur);
            printf("Rouname (%d), brk (%d), ZTrap[0] (%d), dsm2err (%d)\r\n",
                   stack->rouname, stack->brk, stack->ztrap[0], 
                   stack->dsm2err);
            printf("NEW (%d)\r\n",stack->new);
            exit();
          }
#endif
	  stcpy (codptr = code, cmdptr = stack->cmdptr);
#ifdef DEBUG_NEWSTACK
          printf("CHECK 99: (Stack POP)\r\n");
#endif
          tmp_stack = stack; stack = stack->previous;
	  if(stack) forsw = (stack->command == FOR); else forsw = 0;
          free(tmp_stack);
        } else roucur = buff + (NO_OF_RBUF * PSIZE0 + 1);
#else /* NEWSTACK */
        if (nstx == 0)
	    goto restore;		/* nothing to quit */
	if (nestc[nstx] == FOR) {
	    stcpy (code, cmdptr = nestp[nstx--]);
	    codptr = code;
	    ftyp = fortyp[--forx];
	    fvar = forvar[forx];
	    finc = forinc[forx];
	    flim = forlim[forx];
	    fi = fori[forx];
	    if ((forsw = (nestc[nstx] == FOR))) goto for_end;
	    goto next_line;
	}
	if (nestn[nstx]) {		/* reload routine */
	    namptr = nestn[nstx];
	    if ((nestc[nstx] != XECUTE) || loadsw) {
		stcpy (rou_name, namptr);
		zload (rou_name);
		dosave[0] = 0;
	    }
	    namptr--;
	}
	if (nestnew[nstx])
	    unnew ();			/* un-NEW variables */
/* restore old pointers */
	if ((mcmnd = nestc[nstx]) == BREAK)
	    goto restore;		/* cont. single step */
	if (mcmnd == DO_BLOCK) {
	    test = nestlt[nstx];
	    level--;
	}
/* pop $TEST */
	else
	    level = nestlt[nstx];	/* pop level */
	if (nstx) roucur = nestr[nstx] + rouptr;
	else roucur = buff + (NO_OF_RBUF * PSIZE0 + 1);
	stcpy (codptr = code, cmdptr = nestp[nstx--]);
	forsw = (nestc[nstx] == FOR);
#endif
	loadsw = TRUE;
	break;

    case FOR:
	if ((ch = *codptr) == EOL) goto skip_line;	/* ignore empty line */
#ifdef DEBUG_NEWSTACK
        printf("CHECK 03 (Stack PUSH)\r\n"); 
#endif
#ifdef NEWSTACK
        /* Add a stack level */
        tmp_stack = (stack_level *)calloc(1,sizeof(stack_level));
	if (!tmp_stack) { ierr = STKOV; break; } /* Can't do new stack */
	fvar = forvar[++forx];
	finc = forinc[forx];
	flim = forlim[forx];
	fi = fori[forx];
        tmp_stack->command = FOR;
#ifdef DEBUG_NEWSTACK
                if(!cmdptr) printf("CMDPTR is ZERO!\r\n");
#endif          
        tmp_stack->cmdptr  = cmdptr;
        tmp_stack->rouname = NULL;
        tmp_stack->roucur  = roucur - rouptr;
        tmp_stack->ztrap[0]= EOL;
        tmp_stack->previous = stack; 
        tmp_stack->new = NULL;
        stack = tmp_stack;
#else /* NEWSTACK */
	if (++nstx > NESTLEVLS) {
	    nstx--;
	    ierr = STKOV;
	    break;
	}
	fvar = forvar[++forx];
	finc = forinc[forx];
	flim = forlim[forx];
	fi = fori[forx];
	nestc[nstx] = FOR;		/* stack set-up */
#ifdef DEBUG_NEWSTACK
                if(!cmdptr) printf("CMDPTR is ZERO!\r\n");
#endif          
	nestp[nstx] = cmdptr;
	nestn[nstx] = 0;		/* no overring of routine */
	nestr[nstx] = roucur - rouptr;	/* save roucur: only for $V(26) needed */
	ztrap[nstx][0] = EOL;
#endif
	forsw = TRUE;
	ftyp = 0;			/* no args is FOREVER */
	if (ch == SP)
	    goto for_go;
	else {				/* find local variable */
	    if (ch == '^') {
		ierr = GLOBER;
		break;
	    }
	    if (ch == '$') {
		ierr = INVREF;
		break;
	    }
	    if (*(codptr + 1) == '=') {	/* single char local variable */
		if ((ch < 'A' && ch != '%') ||
		    (ch > 'Z' && ch < 'a') || ch > 'z') {
		    ierr = INVREF;
		    break;
		}
		fvar[0] = ch;
		fvar[1] = EOL;
		codptr += 2;
	    } else {
		expr (NAME);
		if (*++codptr != '=')
		    ierr = ASSIGNER;
		if (ierr > OK)
		    break;
		stcpy (fvar, varnam);
		codptr++;
	    }
	    ftyp++;
	}

      for_nxt_arg:

	expr (STRING);
	if (ierr > OK)
	    break;

	stcpy (tmp, argptr);
	if ((ch = *codptr) != ':') {
	    if (ch == ',' || ch == SP || ch == EOL) {
		ftyp = 1;
		goto for_init;
	    }
	    ierr = ARGLIST;
	    break;
	}
	numlit (tmp);			/* numeric interpretation */
	codptr++;
	expr (STRING);
	if (ierr > OK)
	    break;
	numlit (argptr);
	stcpy (finc, argptr);		/* increment */
	if ((ch = *codptr) != ':') {
	    if (ch == ',' || ch == EOL || ch == SP) {
		ftyp = 2;
		goto for_init;
	    }
	    ierr = ARGLIST;
	    break;
	}
	codptr++;
	expr (STRING);
	if (ierr > OK)
	    break;
	numlit (argptr);
	stcpy (flim, argptr);		/* limit */
	ftyp = 3;
	if ((ch = *codptr) != ',' && ch != SP && ch != EOL) {
	    ierr = ARGLIST;
	    break;
	}
	if ((*finc != '-' && comp (flim, tmp)) ||
	    (*finc == '-' && comp (tmp, flim))) {
	    symtab (set_sym, fvar, tmp);
	    if (ierr > OK) {
		stcpy (varerr, vn);
		break;
	    }
	    goto for_quit;
	}
      for_init:

	symtab (set_sym, fvar, tmp);
	if (ierr > OK) {
	    stcpy (varerr, fvar);
	    break;
	}
/* optimize frequent special case: */
/* increment by one and no additional FOR arguments */
/* if limit value it must be a positive integer */

	if (ftyp > 1 && finc[0] == '1' && finc[1] == EOL) {
	    j = TRUE;
	    if (ftyp == 3) {
		i = 0;
		while ((ch = flim[i]) != EOL) {
		    if (ch < '0' || ch > '9')
			j = FALSE;
		    i++;
		}
		fi = i;
		fori[forx] = i;
	    }
	    if (j && ((ch = *codptr) == SP || ch == EOL)) {
		ftyp += 2;
		if (ch == SP)
		    codptr++;
	    }
	}
      for_go:
	fortyp[forx] = ftyp;
#ifdef NEWSTACK
        if(!stack) { printf("BAIL 01\r\n"); exit(1); }
        stack->cmdptr = cmdptr;
#else
#ifdef DEBUG_NEWSTACK
                if(!cmdptr) printf("CMDPTR is ZERO!\r\n");
#endif          
	nestp[nstx] = cmdptr;
#endif
	cmdptr += stcpy (cmdptr, codptr) + 1;
	if (ftyp > 3)
	    goto next_cmnd;

/* skip following for arguments if there are any */
      for10:
	if (*codptr == SP)
	    goto next_cmnd;
	i = 0;
	while ((((ch = *codptr) != SP) || i) && ch != EOL) {
	    if (ch == '"')
		i = !i;
	    codptr++;
	}				/* skip rest of FOR list */
	goto next_cmnd;

      for_end:				/* end of line return */
#ifdef NEWSTACK
        if(!stack) { printf("BAIL 02\r\n"); exit(1); } 
	stcpy (codptr = code, stack->cmdptr);	/* restore old pointers */
#else
#ifdef DEBUG_NEWSTACK
        printf("For_end: nstx: %d, Nestp: (%d)\r\n",nstx, nestp[nstx]);
#endif
	stcpy (codptr = code, nestp[nstx]);	/* restore old pointers */
#endif

	switch (ftyp) {
	case 5:			/* frequent special case: increment 1 */
	    symtab (getinc, fvar, tmp);

/*  compare fvar-value to flim-value */
/* fi: i=0; while (flim[i]>='0') i++; */
/* Note: EOL<'-'<'.'<'0' tmp has at least one character */
	    ch = '0';
	    j = 1;
	    while (tmp[j] >= ch)
		j++;
	    if (j < fi)
		goto next_cmnd;
	    if (j == fi) {
		j = 0;
		while (tmp[j] == flim[j]) {
		    if (tmp[j] == EOL)
			goto next_cmnd;
		    j++;
		}
		if (tmp[j] <= flim[j])
		    goto next_cmnd;
	    }
	    if (flim[0] != '-' && tmp[0] == '-')
		goto next_cmnd;

	    stcpy (tmp2, "-1\201");	/* correct last inc */
	    add (tmp, tmp2);
	    symtab (set_sym, fvar, tmp);
	    goto for_quit;


	case 4:			/* frequent special case: increment 1 without limit */
	    symtab (getinc, fvar, tmp);
	case 0:
	    goto next_cmnd;

	case 3:			/* FOR with increment and limit test */
	    symtab (get_sym, fvar, tmp);
	    numlit (tmp);
	    stcpy (tmp2, finc);		/* add may change forinc */
	    add (tmp, tmp2);

	    if (*finc != '-') {
		if (comp (flim, tmp))
		    goto for_quit;
	    } else {
		if (comp (tmp, flim))
		    goto for_quit;
	    }

	    symtab (set_sym, fvar, tmp);
	    goto for10;

	case 2:			/* FOR with increment without limit test */
	    symtab (get_sym, fvar, tmp);
	    numlit (tmp);
	    stcpy (tmp2, finc);		/* add may change forinc */
	    add (tmp, tmp2);

	    symtab (set_sym, fvar, tmp);
	    goto for10;
	}				/* end switch */

      for_quit:
#ifdef NEWSTACK
        if(!stack) { printf("BAIL 03\r\n"); exit(1); }
        else { cmdptr = stack->cmdptr; }
#else
	cmdptr = nestp[nstx];
#endif
	if (*codptr++ == ',')
	    goto for_nxt_arg;
#ifdef NEWSTACK
        if(!stack) { printf("BAIL 04\r\n"); exit(1); }
           tmp_stack = stack; stack = stack->previous; free(tmp_stack);
           if(!stack) forsw = 0;
        
#else
	nstx--;
#endif
	forx--;
	ftyp = fortyp[forx];
	fvar = forvar[forx];
	finc = forinc[forx];
	flim = forlim[forx];
	fi = fori[forx];
#ifdef NEWSTACK
	if (stack && (forsw = (stack->command == FOR))) goto for_end;
#else
	if ((forsw = (nestc[nstx] == FOR))) goto for_end;
#endif
	goto next_line;

    case MERGE:
	expr (NAME);
	if (ierr > OK)
	    break;
	stcpy (vn, varnam);
	if (*++codptr != '=') {
	    ierr = ASSIGNER;
	    break;
	}
	codptr++;
	expr (NAME);
	if (ierr > OK)
	    break;
	codptr++;
	if (vn[0] == '^')
	    global  (merge_sym, vn, argptr);

	else
	    symtab (merge_sym, vn, varnam);
	if (ierr > OK)
	    stcpy (varerr, vn);
	break;

    case RLOAD: 
        stcpy(&tmp3[1],"zrload \201"); 
        goto private;
    case RSAVE: 
    	stcpy(&tmp3[1],"zrsave \201"); 
    	goto private;
    case XECUTE:

	expr (STRING);
	if (ierr > OK)
	    break;
	stcpy (tmp, argptr);

	if (*codptr == ':') {		/* argument postcond */
	    codptr++;
	    expr (STRING);
	    if (ierr > OK)
		break;
	    if (tvexpr (argptr) == FALSE)
		break;
	}
#ifdef NEWSTACK
        /* New Stack level */
#ifdef DEBUG_NEWSTACK
        printf("CHECK 04\r\n"); 
#endif
        tmp_stack = (stack_level *)calloc(1,sizeof(stack_level));
	if (!tmp_stack) { ierr = STKOV; break; } /* Can't do new stack */
        tmp_stack->command = XECUTE;
#ifdef DEBUG_NEWSTACK
                if(!cmdptr) printf("CMDPTR is ZERO!\r\n");
#endif          
        tmp_stack->cmdptr  = cmdptr;
        tmp_stack->roucur  = roucur - rouptr;
        tmp_stack->level   = level; level = 0; 
        /* Next line leaks */
        tmp_stack->rouname = (char *)calloc(stlen(rou_name)+1,sizeof(char));
        stcpy(tmp_stack->rouname, rou_name);
        tmp_stack->ztrap[0] = EOL;
        tmp_stack->previous = stack; 
        tmp_stack->new = NULL;
        stack = tmp_stack;
#else /* NEWSTACK */
        if (++nstx > NESTLEVLS) { nstx--; ierr = STKOV; break; }
	nestc[nstx] = XECUTE;
#ifdef DEBUG_NEWSTACK
                if(!cmdptr) printf("CMDPTR is ZERO!\r\n");
#endif          
	nestp[nstx] = cmdptr;		/* command stack address */
	nestr[nstx] = roucur - rouptr;	/* save roucur */
	nestlt[nstx] = level;
	level = 0;			/* save level */
	nestnew[nstx] = 0;
	ztrap[nstx][0] = EOL;
	while ((*(namptr++)) != EOL) ;
	stcpy ((nestn[nstx] = namptr), rou_name);  /* save routine name */
#endif
	forsw = FALSE;
	loadsw = FALSE;
	cmdptr += stcpy (cmdptr, codptr) + 1;
	stcpy (code, tmp);
	roucur = buff + (NO_OF_RBUF * PSIZE0 + 1);
	codptr = code;
	goto next_cmnd;

    case DO:

	rouoldc = roucur - rouptr;
	namold = 0;

    case GOTO:

      do_goto:;

	offset = 0;
	label[0] = routine[0] = EOL;
	dofram0 = 0;

	if (((ch = *codptr) != '+') && (ch != '^')) {	/* parse label */
	    if (ch == SP || ch == EOL) {	/* no args: blockstructured DO */
		if (mcmnd != DO) {
		    ierr = ARGLIST;
		    break;
		}
/* direct mode: DO +1 */
#ifdef NEWSTACK
		if (!stack && roucur >= rouend) {
#else
		if (nstx == 0 && roucur >= rouend) {
#endif
		    roucu0 = rouptr;
		    goto off1;
		}
		mcmnd = DO_BLOCK;
		roucu0 = roucur;	/* continue with next line */
		forsw = FALSE;
		goto off2;
	    }
	    expr (LABEL);
	    if (ierr > OK)
		goto err;
	    stcpy (label, varnam);
	    ch = *++codptr;
	}
	if (ch == '+') {		/* parse offset */
	    codptr++;
	    expr (OFFSET);
	    if (ierr > OK)
		goto err;
	    offset = intexpr (argptr);
	    dosave[0] = EOL;
/* unless argument is numeric, expr returns wrong codptr */
	    if ((ch = *codptr) != SP && (ch != EOL) && (ch != ',') && (ch != '^'))
		ch = *++codptr;
	}
	if (ch == '^') {		/* parse routine */
	    codptr++;
	    expr (LABEL);
	    if (ierr > OK)
		goto err;
	    stcpy (routine, varnam);
	    dosave[0] = EOL;
	    ch = *++codptr;
	    loadsw = TRUE;
	}
	if (ch == '(' && mcmnd == DO) {	/* parse parameter */
	    if (offset) {
		ierr = ARGLIST;
		goto err;
	    }
	    if (*++codptr == ')')
		ch = *++codptr;
	    else {
		dofram0 = dofrmptr;
		i = 0;
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
			goto err;
		    }
		    ch = *codptr++;
		    if (ch == ',')
			continue;
		    if (ch != ')') {
			ierr = COMMAER;
			dofrmptr = dofram0;
			goto err;
		    }
		    ch = *codptr;
		    break;
		}
	    }
	}
	if (ch == ':') {		/* parse postcond */
	    codptr++;
	    expr (STRING);
	    if (ierr > OK) {
		if (dofram0)
		    dofrmptr = dofram0;	/* reset frame pointer */
		goto err;
	    }
	    if (tvexpr (argptr) == FALSE) {
		if (*codptr != ',')
		    mcmnd = 0;		/* avoid false LEVEL Error */
		if (dofram0)
		    dofrmptr = dofram0;	/* reset frame pointer */
		break;
	    }
	}
	if (mcmnd == GOTO) {		/* GOTO: clear FORs from stack */
#ifdef DEBUG_NEWSTACK
            printf("CHECK 05 Multi-POP on FOR\r\n"); 
#endif
#ifdef NEWSTACK
            while(stack && stack->command == FOR) {
              cmdptr = stack->cmdptr;
              tmp_stack = stack; stack=stack->previous; free(tmp_stack);
              /* if(!stack) { printf ("PANIC CHECK 05\r\n"); }
                 else cmdptr = stack->cmdptr; */
#ifdef DEBUG_NEWSTACK
              printf("POP");
#endif
              forx--;
	      ftyp = fortyp[forx];
	      fvar = forvar[forx];
	      finc = forinc[forx];
	      flim = forlim[forx];
	      fi = fori[forx];
            }
            printf("\r\n");
#else /* NEWSTACK */
	    while (nestc[nstx] == FOR) {
#ifdef DEBUG_NEWSTACK
                printf("POP");
#endif
		cmdptr = nestp[nstx--];
		forx--;
		ftyp = fortyp[forx];
		fvar = forvar[forx];
		finc = forinc[forx];
		flim = forlim[forx];
		fi = fori[forx];
	    }
#ifdef DEBUG_NEWSTACK
                printf("\r\n");
#endif
#endif /* NEWSTACK */
	    loadsw = TRUE;
	}
      job_entry:;			/* entry called from successful JOB */
	if (routine[0] != EOL) {
#ifdef DEBUG_NEWSTACK
              printf("CHECK 06\r\n"); 
#endif
	    if (mcmnd == DO) {
		while ((*(namptr++)) != EOL) ;
		namold = namptr;
		stcpy (namptr, rou_name);
	    }
/* if (GOTO label^rou) under a (DO label)   */
/* save away old routine to restore on quit */
#ifdef NEWSTACK
            else if (stack && stack->previous) {
#ifdef DEBUG_NEWSTACK
              printf("Stack previous exists in CHECK 06\r\n"); 
#endif
              while(stack && (stack->command == FOR)) {
#ifdef DEBUG_NEWSTACK
              printf("POP"); 
#endif
		    forx--;
		    ftyp = fortyp[forx];
		    fvar = forvar[forx];
		    finc = forinc[forx];
		    flim = forlim[forx];
		    fi = fori[forx];
                    tmp_stack = stack; stack = stack->previous; 
                    free(tmp_stack);
	      }
              if(!stack->command) { 
                stack->rouname = (char *)calloc(stlen(rou_name)+1,
                                                sizeof(char));
                stcpy(stack->rouname, rou_name);
              }
            }
#else /* NEWSTACK */
	    else if (nstx > 0) {
#ifdef DEBUG_NEWSTACK
              printf("CHECK 06, stack is greater than 0\r\n"); 
#endif
		while (nestc[nstx] == FOR) {
#ifdef DEBUG_NEWSTACK
              printf("POP"); 
#endif
		    nstx--;
		    forx--;
		    ftyp = fortyp[forx];
		    fvar = forvar[forx];
		    finc = forinc[forx];
		    flim = forlim[forx];
		    fi = fori[forx];
		}
		if (nestn[nstx] == 0) {
		    while ((*(namptr++)) != EOL) ;
		    stcpy ((nestn[nstx] = namptr), rou_name);
		}
	    }
#endif
	    zload (routine);
	    if (ierr > OK)
		goto err;		/* load file */
	} {
	    char   *reg,
	           *reg1;

	    reg1 = rouptr;
	    reg = reg1;
	    if (label[0] != EOL) {
		if (forsw && mcmnd == DO && stcmp (label, dosave) == 0) {
		    roucu0 = xdosave;
		    goto off1;
		}
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
		    goto err;
		}
	    }
	  off:
	    if (label[0] == EOL && offset > 0)
		offset--;
	    while (offset-- > 0)
		reg1 = reg1 + (UNSIGN (*reg1) + 2);
	    if (forsw) {
		xdosave = reg1;
		stcpy (dosave, label);
	    }
	    roucu0 = reg1;
	}
	if (roucu0 >= rouend) {
	    ierr = LBLUNDEF;
	    stcpy (varerr, label);	/* to be included in error message */
	    if (dofram0)
		dofrmptr = dofram0;	/* reset frame pointer */
	    zload (rou_name);
	    goto err;
	}
      off1:
	if (routine[0] != EOL)
	    stcpy (rou_name, routine);
	roucu0++;
	forsw = FALSE;
	if (mcmnd != DO) {		/* i.e. GOTO or JOB */
	    roucur = roucu0;
	    goto off3;
	}
      off2:
#ifdef DEBUG_NEWSTACK
        printf("CHECK 07 (Stack PUSH)\r\n"); 
#endif
#ifdef NEWSTACK
        tmp_stack = (stack_level *)calloc(1,sizeof(stack_level));
        if(!tmp_stack) { ierr = STKOV; goto err; }
        tmp_stack->command = mcmnd;
#ifdef DEBUG_NEWSTACK
                if(!cmdptr) printf("CMDPTR is ZERO!\r\n");
#endif          
        tmp_stack->cmdptr  = cmdptr;
        tmp_stack->rouname = namold;
        tmp_stack->roucur  = rouoldc;  
        if(mcmnd != DO_BLOCK) { tmp_stack->level = level; level = 0; }
        else { tmp_stack->level = test; level++; }
        tmp_stack->ztrap[0] = EOL;

        tmp_stack->previous = stack; tmp_stack->new = NULL;
        stack = tmp_stack;
#else  /* NEWSTACK */
        if (++nstx > NESTLEVLS) {
	    nstx--;
	    ierr = STKOV;
	    goto err;
	}
	nestc[nstx] = mcmnd;
#ifdef DEBUG_NEWSTACK
                if(!cmdptr) printf("CMDPTR is ZERO!\r\n");
#endif          
	nestp[nstx] = cmdptr;
	nestn[nstx] = namold;
	nestr[nstx] = rouoldc;
	nestnew[nstx] = 0;
	if (mcmnd != DO_BLOCK) {
	    nestlt[nstx] = level;
	    level = 0;
	}
/* push level ; clr level */
	else {
	    nestlt[nstx] = test;
	    level++;
	}				/* push $TEST ; inc level */
	ztrap[nstx][0] = EOL;
#endif

	cmdptr += stcpy (cmdptr, codptr) + 1;
	roucur = roucu0;
/* processing for private Z-Command: */
	if (privflag) {
#ifndef NEWSTACK
#ifdef DEBUG_NEWPTR
	    printf("Xecline 01 (using NEWPTR): ");
	    printf("[nstx] is [%d], [nestnew] is [%d]",nstx,nestnew[nstx]);
            printf("- Initialized to newptr\r\n");
#endif /* Debug */
	    nestnew[nstx] = newptr; 
#endif /* NEWSTACK */
	    stcpy(vn,zargdefname);

/*was: 	    vn[0] = '%';   vn[1] = EOL; */

	    symtab (new_sym, vn, "");
/*djw change 'input variable for Z command' to get value of $V(202) */
/*was:	    vn[0] = '%';   vn[1] = EOL; */

	    stcpy(vn,zargdefname);
	    symtab (set_sym, vn, tmp2);
	    privflag = FALSE;
	}
      off3:if (dofram0) {
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
		goto err;
	    }
	    j = 0;
	    while ((ch = (*reg++)) != EOL) {
		if ((ch == ',' && j) || ch == ')') {
		    varnam[j] = EOL;
#ifndef NEWSTACK
#ifdef DEBUG_NEWPTR
	 printf("Xecline 02: ");
	 printf("[nstx] is [%d], [nestnew] is [%d]\r\n",nstx,nestnew[nstx]);
#endif
		    if (nestnew[nstx] == 0)
			nestnew[nstx] = newptr;
#endif /* NEWSTACK */
		    if (reg1 < dofrmptr) {
			if (*reg1 == DELIM) {	/* call by reference */
			    if (stcmp (reg1 + 1, varnam)) {	/* are they different?? */
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
		dofrmptr = dofram0;	/* reset frame pointer */
		goto err;
	    }
	    if (reg1 < dofrmptr) {
		ierr = TOOPARA;
		dofrmptr = dofram0;
		goto err;
	    }
	    dofrmptr = dofram0;
	}
	goto next_line;

/* ZJOB *//* same as JOB, but without timeout */
/* not recommended; just for backward compatibility */
    case ZJOB:
	if (standard) {
	    ierr = NOSTAND;
	    goto err;
	}
    case JOB:

	if (*codptr == SP || *codptr == EOL) {
	    ierr = LBLUNDEF;
	    varerr[0] = EOL;		/* to be included in error message */
	    break;
	}
	loadsw = TRUE;
	offset = 0;
	timeout = (-1L);
	label[0] = routine[0] = EOL;
	if (((ch = *codptr) != '+') && (ch != '^')) {	/* parse label */
	    expr (LABEL);
	    if (ierr > OK)
		goto err;
	    stcpy (label, varnam);
	    ch = *++codptr;
	}
	if (ch == '+') {		/* parse offset */
	    codptr++;
	    expr (OFFSET);
	    if (ierr > OK)
		goto err;
	    offset = intexpr (argptr);
/* unless argument is numeric, expr returns wrong codptr */
	    if ((ch = *codptr) != SP && (ch != EOL) && (ch != ',') && (ch != '^'))
		ch = *++codptr;
	}
	if (ch == '^') {		/* parse routine */
	    codptr++;
	    expr (LABEL);
	    if (ierr > OK)
		goto err;
	    stcpy (routine, varnam);
	    dosave[0] = EOL;
	    ch = *++codptr;
	}
	dofram0 = NULL;
	if (ch == '(') {		/* parse parameter */
	    if (offset) {
		ierr = ARGLIST;
		goto err;
	    }
	    codptr++;
	    dofram0 = dofrmptr;
	    i = 0;
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
		    goto err;
		}
		ch = *codptr++;
		if (ch == ',')
		    continue;
		if (ch != ')') {
		    ierr = COMMAER;
		    dofrmptr = dofram0;
		    goto err;
		}
		ch = *codptr;
		break;
	    }
	}
	if (ch == ':' && *(codptr + 1) == ch)
	    codptr++;			/* timeout,no jobparams */
/* parse any 'job parameters', but ignore them otherwise */
	else if (ch == ':' && *(codptr + 1) == '(') {
	    codptr++;
	    setpiece = TRUE;		/* to avoid bracket error at end of jobparameters */
	    for (;;)
	    {
		if (*++codptr != ':')
		    expr (STRING);
		if (*codptr == ':')
		    continue;
		if (*codptr++ != ')')
		    ierr = ARGER;
		break;
	    }
	    setpiece = FALSE;
	    ch = (*codptr);
	}
	if (ch == ':') {		/* timeout */
	    codptr++;
	    expr (STRING);
	    if ((timeout = intexpr (argptr)) < 0L)
		timeout = 0L;
	    if (ierr > OK)
		goto err;
	    test = TRUE;
	}
	if (mcmnd == ZJOB)
	    timeout = 0L;		/* ZJOB-command has timeout 0 */

	close_all_globals ();			/* close all globals */
	j = getpid ();			/* job number of father process */
	if (lonelyflag) {		/* single user */
	    if (timeout < 0L)
		ierr = PROTECT;		/* error without timeout */
	    else
		test = FALSE;		/* timeout always fails */
	    break;
	}
	while ((i = fork ()) == -1) {
	    if (timeout == 0L) {
		test = FALSE;
		break;
	    }
	    if (timeout > 0L)
		timeout--;
	    sleep (1);
	}
	if (mcmnd == ZJOB && zjobflag) {
	    if (i == 0) {		/* we are in child process */
		intstr (zb, j);		/* $JOB of father job */
		father = j;
		pid = getpid ();	/* this is our new job number */

		jobtime = time (0L);;
#ifdef NEWSTACK
#ifdef DEBUG_NEWSTACK
                printf("Check 98 (Stack MegaPOP)\r\n");
#endif
                while(stack) {
#ifdef DEBUG_NEWSTACK
                printf("POP");
#endif
                  tmp_stack = stack; stack=stack->previous;
                  free(tmp_stack);
                }
#else
		nstx = 0;		/* clear stack */
#endif
		forx = 0;
		forsw = FALSE;
		level = 0;
		cmdptr = cmdstack;	/*  -  command stack pointer */
		namptr = namstck;	/*  -  routine name stack pointer */
		usermode = 0;		/* application mode */
		ierr = OK;
		lock (" \201", -1, 'j');	/* tell lock about JOB */
		goto job_entry;
	    }
						/* ignore signal while here */
	    SIGNAL_ACTION(SIGUSR1, SIG_IGN, NULL);

	    while (wait (&zsystem) != i) ;

	    SIGNAL_ACTION(SIGUSR1, oncld, NULL);/* restore handler */

	    ierr = OK;			/* there might be a INRPT from other job */
	    set_io (MUMPS);
	    break;
	}
	if (i == 0) {			/* we are in child process */
	    intstr (zb, j);		/* $JOB of father job */
	    father = j;

	    pid = getpid ();		/* $J = process ID */
	    usermode = 0;		/* no programmer mode */
	    DSW |= BIT0;		/* disable echo */
	    zbreakon = DISABLE;		/* disable CTRL/B */
	    breakon = DISABLE;		/* disable CTRL/C */
	    hardcopy = DISABLE;		/* disable hardcopy function */
	    fclose (stdin);		/* close normal input */
	    jour_flag = 0;		/* no protocol */
#ifdef NEWSTACK
#ifdef DEBUG_NEWSTACK
            printf("Check 97 Stack MegaPOP\r\n"); 
#endif
            while(stack) {
              tmp_stack = stack; stack = stack->previous; free(tmp_stack);
            }
#else
	    nstx = 0;			/* clear stack */
#endif
	    forx = 0;
	    forsw = FALSE;
	    level = 0;
	    cmdptr = cmdstack;		/*  -  command stack pointer */
	    namptr = namstck;		/*  -  routine name stack pointer */
/* init random number */
	    if ((random = time (0L) * getpid ()) < 0)
		random = (-random);
	    ierr = OK;
	    lock (" \201", -1, 'j');	/* tell lock about JOB */
	    goto job_entry;
	}
	intstr (zb, i);			/* $JOB of the process just started */
	break;

    case KILL:

/* argumentless: KILL all local variables */
	if (((ch = *codptr) == SP) || ch == EOL) {
	    symtab (kill_all, "", "");
	    break;
	}
	if (ch != '(') {
	    expr (NAME);
/* aviod a disaster if someone types KILL ^PATDAT[TEST] ! */
	    if (((ch = *++codptr) != SP) && ch != EOL && ch != ',')
		ierr = INVREF;
	    if (ierr > OK)
		goto err;
	    if (varnam[0] == '^') {
		global  (kill_sym, varnam, tmp);

		break;
	    }
	    symtab (kill_sym, varnam, tmp);
	    break;
	}
/* exclusive kill */
	tmp[0] = SP;
	tmp[1] = EOL;
	for (;;)
	{
	    codptr++;
	    expr (NAME);
	    if (ierr > OK)
		goto err;
	    if (varnam[0] == '^') {
		ierr = GLOBER;
		goto err;
	    }
	    i = 0;
	    while (varnam[i] != EOL) {
		if (varnam[i] == DELIM) {
		    ierr = SBSCR;
		    goto err;
		}
		i++;
	    }
	    if (stcat (tmp, varnam) == 0) {
		ierr = MXSTR;
		goto err;
	    }
	    if (stcat (tmp, " \201") == 0) {
		ierr = MXSTR;
		goto err;
	    }
	    if ((ch = *++codptr) == ')') {
		codptr++;
		break;
	    }
	    if (ch != ',') {
		ierr = COMMAER;
		goto err;
	    }
	}
	symtab (killexcl, tmp, "");
	break;
    case NEWCMD:
    case ZNEW:

/* argumentless: NEW all local variables */
	if (((ch = *codptr) == SP) || ch == EOL) {
	    ch = nstx;
	    while (nestc[ch] == FOR)
		ch--;			/* FOR does not define a NEW level */
#ifdef DEBUG_NEWPTR
	    printf("Xecline 03: (TODO - NEW ALL) ");
            printf("[ch] is %d, [nestnew] is %d\r\n",ch, nestnew[ch]);
#endif
	    if (nestnew[ch] == 0) nestnew[ch] = newptr;
	    symtab (new_all, "", "");
	    break;
	}
	if (ch != '(') {
	    expr (NAME);
	    if (ierr > OK)
		goto err;
	    codptr++;
	    if (varnam[0] == '^') {
		ierr = GLOBER;
		goto err;
	    }
	    if (varnam[0] == '$') {
		i = 0;
		while ((ch = varnam[++i]) != EOL)
		    if (ch >= 'A' && ch <= 'Z')
			varnam[i] = ch + 32;
		if ((stcmp (&varnam[1], "r\201")) &&	/* set $reference */
		    (stcmp (&varnam[1], "reference\201")) &&
		    (stcmp (&varnam[1], "zr\201")) &&	/* set $reference */
		    (stcmp (&varnam[1], "zreference\201")) &&
		    (stcmp (&varnam[1], "t\201")) &&
		    (stcmp (&varnam[1], "test\201")) &&
		    (stcmp (&varnam[1], "j\201")) &&
		    (stcmp (&varnam[1], "job\201")) &&
		    (stcmp (&varnam[1], "zi\201")) &&
		    (stcmp (&varnam[1], "zinrpt\201"))) {
		    ierr = INVREF;
		    goto err;
		}
	    }
#ifdef NEWSTACK
#ifdef DEBUG_NEWSTACK
	    printf("Xecline 04 (SKIPPED implemented in symtab.c instead):\r\n");
#endif /* DEBUG */
#if 0
            tmp_stack = stack;
            while(tmp_stack && tmp_stack->command == FOR) {
              printf("04 BOING!\r\n");
              tmp_stack = tmp_stack->previous;
            }
            if(!tmp_stack->new) tmp_stack->new = stack->new;
#endif
#else /* NEWSTACK */
	    ch = nstx;
	    while (nestc[ch] == FOR)
		ch--;			/* FOR does not define a NEW level */
#ifdef DEBUG_NEWPTR
	    printf("Xecline 04 (DANGER): ");
            printf("[ch] is %d, [nestnew] is %d\r\n",ch, nestnew[ch]);
#endif
	    if (nestnew[ch] == 0) nestnew[ch] = newptr;
#endif /* NEWSTACK */
	    symtab (new_sym, varnam, "");
	    break;
	}
/* exclusive new */
	tmp[0] = SP;
	tmp[1] = EOL;
	for (;;)
	{
	    codptr++;
	    expr (NAME);
	    if (ierr > OK)
		goto err;
	    if (varnam[0] == '^') {
		ierr = GLOBER;
		goto err;
	    }
	    if (varnam[0] == '$') {
		ierr = INVREF;
		goto err;
	    }
	    i = 0;
	    while (varnam[i] != EOL) {
		if (varnam[i] == DELIM) {
		    ierr = SBSCR;
		    goto err;
		}
		i++;
	    }
	    if (stcat (tmp, varnam) == 0) {
		ierr = MXSTR;
		goto err;
	    }
	    if (stcat (tmp, " \201") == 0) {
		ierr = MXSTR;
		goto err;
	    }
	    if ((ch = *++codptr) == ')') {
		codptr++;
		break;
	    }
	    if (ch != ',') {
		ierr = COMMAER;
		goto err;
	    }
	}
	ch = nstx;
	while (nestc[ch] == FOR)
	    ch--;			/* FOR does not define a NEW level */
#ifdef DEBUG_NEWPTR
	    printf("Xecline 05 (TODO): ");
            printf("[ch] is %d, [nestnew] is %d\r\n",ch, nestnew[ch]);
#endif
	if (nestnew[ch] == 0) nestnew[ch] = newptr;

	symtab (newexcl, tmp, "");
	break;

    case LOCK:

/* argumentless: UNLOCK */
	if ((ch = *codptr) == SP || ch == EOL) {
	    lock (" \201", -1, LOCK);
	    break;
	}
	if (ch == '+' || ch == '-') {
	    tmp[0] = ch;
	    ch = (*++codptr);
	} else
	    tmp[0] = SP;
	if (ch != '(') {
	    expr (NAME);
	    if (ierr > OK)
		goto err;
	    stcpy (&tmp[1], varnam);
	    stcat (tmp, "\001\201");
	} else {			/* multiple lock */
	    tmp[1] = EOL;
	    for (;;)
	    {
		codptr++;
		expr (NAME);
		if (ierr > OK)
		    goto err;
		stcat (tmp, varnam);
		stcat (tmp, "\001\201");
		if ((ch = *++codptr) == ')')
		    break;
		if (ch != ',') {
		    ierr = COMMAER;
		    goto err;
		}
	    }
	}
	timeout = (-1L);		/* no timeout */
	if (*++codptr == ':') {
	    codptr++;
	    expr (STRING);
	    timeout = intexpr (argptr);
	    if (ierr > OK)
		goto err;
	    if (timeout < 0L)
		timeout = 0L;
	}
	lock (tmp, timeout, LOCK);
	break;

    case USE:

	if (*codptr == SP || *codptr == EOL) {
	    ierr = ARGER;
	    goto err;
	}
	expr (STRING);
	j = intexpr (argptr);
	if (j < 0 || j > MAXDEV)
	    ierr = NODEVICE;
	else if (j != HOME && devopen[j] == 0)
	    ierr = NOPEN;
	if (ierr > OK)
	    goto err;
	io = j;
	if (io == HOME && *codptr == ':' && *(codptr + 1) == '(') {
	  use0:;			/* entry point for processing of device parameters */
	    codptr += 2;
	    j = 1;
	    setpiece = TRUE;		/* so a surplus closing bracket will not be an error */
	    while (*codptr != ')') {
		if (*codptr == ':') {
		    codptr++;
		    j++;
		    continue;
		}
		expr (STRING);
		if (ierr > OK) {
		    setpiece = FALSE;
		    goto err;
		}
		switch (j) {
		case 1:
		    i = intexpr (argptr);
		    if (i < 0)
			i = 0;
		    if (i > 255)
			i = 255;
		    RightMargin = i;
		    break;
		case 3:
		    i = intexpr (argptr);
		    if (i < 0)
			i = 0;
		    if (i > 255)
			i = 255;
		    InFieldLen = i;
		    break;
		case 5:
		    DSW = intexpr (argptr);
		    break;
		case 7:
		    i = intexpr (argptr);
		    ypos[HOME] = i / 256;
		    xpos[HOME] = i % 256;
		    if (DSW & BIT7) {
			i = io;
			io = HOME;
			argptr[0] = ESC;
			argptr[1] = '[';
			argptr[2] = EOL;
			if (ypos[HOME]) {
			    intstr (&argptr[2], ypos[HOME] + 1);
			}
			if (xpos[HOME]) {
			    tmp3[0] = ';';
			    intstr (&tmp3[1], xpos[HOME] + 1);
			    stcat (argptr, tmp3);
			}
			stcat (argptr, "H\201");
			write_m (argptr);
			io = i;
		    }
		    break;
		case 9:
		    i = 0;
		    j = 0;
		    while ((ch = argptr[i++]) != EOL)
			LineTerm[j++] = ch;
		    LineTerm[j] = EOL;
		    break;
		case 10:
		    BrkKey = (*argptr);
/* make new break active */
		    set_io (UNIX);
		    set_io (MUMPS);
		}
	    }
	    setpiece = FALSE;
	    codptr++;
	    break;
	} else if (*codptr == ':') {
	    codptr++;
	    if (io == HOME) {		/* old syntax: enable/disable echo */
		expr (STRING);
		if (ierr > OK)
		    goto err;
		if (tvexpr (argptr))
		    DSW &= ~BIT0;
		else
		    DSW |= BIT0;
	    } else {
		if (*codptr == '(') {
		    codptr++;
		    setpiece = TRUE;
		}
		j = 1;
		while (*codptr != ')') {
		    if (*codptr == ':') {
			codptr++;
			j++;
			continue;
		    } else if (setpiece == FALSE) {
			ierr = SPACER;
			goto err;
		    }
		    expr (STRING);
		    if (ierr > OK) {
			setpiece = FALSE;
			goto err;
		    }
		    switch (j) {
		    case 1:
			fseek (opnfile[io], (long) intexpr (argptr), 0);
			break;
		    case 2:
			crlf[io] = tvexpr (argptr);
			break;
		    case 3:
			nodelay[io] = tvexpr (argptr);
			break;
		    }
		    if (setpiece == FALSE)
			break;
		}
		if (setpiece) {
		    codptr++;
		    setpiece = FALSE;
		}
		break;
	    }
	}
	break;

    case OPEN:

	{
	    short   k;

	    if (*codptr == SP || *codptr == EOL) {
		ierr = FILERR;
		goto err;
	    }
	    expr (STRING);
	    k = intexpr (argptr);
	    if (ierr > OK)
		goto err;

	    if (k < 0 || k > MAXDEV) {
		ierr = NODEVICE;
		goto err;
	    }
	    crlf[k] = FALSE;
	    nodelay[k] = FALSE;
	    xpos[k] = 0;
	    ypos[k] = 0;

/* OPEN implies a previous CLOSE on same channel */
	    if ((k != HOME) && devopen[k]) {
		fclose (opnfile[k]);
		devopen[k] = 0;
		if (io == k)
		    io = HOME;
	    }
/* process device parameters on HOME at USE command. */
	    if (k == HOME && *codptr == ':' && *(codptr + 1) == '(')
		goto use0;
	    if (*codptr != ':') {
		if (k == HOME)
		    break;
		if (dev[k][0] == EOL) {
		    ierr = FILERR;
		    goto err;
		}
		goto open10;
	    }
	    codptr++;
	    if (k == HOME) {
		if (*codptr != ':') {	/* turn echo on/off */
		    expr (STRING);
		    if (ierr > OK)
			goto err;
		    if (tvexpr (argptr))
			DSW &= ~BIT0;
		    else
			DSW |= BIT0;
		}
		if (*codptr == ':') {	/* dummy timeout on HOME */
		    codptr++;
		    expr (STRING);
		    if (ierr > OK)
			goto err;
		    test = TRUE;
		}
	    } else {
		int     op_pos;

		expr (STRING);
		if (ierr > OK)
		    goto err;
		stcpy (dev[k], argptr);
		timeout = (-1L);
		if (*codptr == ':') {
		    codptr++;
		    expr (STRING);
		    timeout = intexpr (argptr);
		    if (ierr > OK)
			goto err;
		    if (timeout < 0L)
			timeout = 0L;
		}
	      open10:;
		j = stcpy (tmp, dev[k]);
		i = dev[k][j - 1];
		while (--j >= 0) {
		    if (dev[k][j] == '/')
			break;
		}
		stcpy (tmp2, dev[k]);
		if (j <= 0) {
		    tmp2[stlen (tmp2)] = NUL;
		    tmp[1] = 'r';
		    i = '+';
		}
/* default is read+write */
		else {
		    tmp2[j] = NUL;
		    j = stcpy (&tmp[1], &tmp[j + 1]);
		    tmp[0] = SP;
		    tmp[j + 1] = SP;
		    tmp[j + 2] = EOL;
		    j = 0;
		    while ((ch = tmp[++j]) != EOL)
			if (ch >= 'A' && ch <= 'Z')
			    tmp[j] = ch + 32;
		    if (find (" r w a r+ w+ a+ read write append read+ write+ append+ \201", tmp) == FALSE) {
			tmp[1] = 'r';
			i = '+';
			tmp2[strlen (tmp2)] = '/';
		    }
		}
		tmp[0] = tmp[1];
		tmp[1] = NUL;		/* NUL not EOL !!! */
		if (i == '+') {
		    tmp[1] = i;
		    tmp[2] = NUL;
		}
		op_pos = 0;
	      open20:;
		if (oucpath[op_pos] != EOL) {
		    j = stlen (dev[k]);
		    while (--j >= 0) {
			if (dev[k][j] == '/')
			    break;
		    }
		    while (--j >= 0) {
			if (dev[k][j] == '/')
			    break;
		    }
		    if (j < 0) {
			strcpy (tmp3, tmp2);
			stcpy (tmp2, &oucpath[op_pos]);
			j = 0;
			while (tmp2[j] != ':' && tmp2[j] != EOL)
			    j++;
			tmp2[j] = EOL;
			stcpy (act_oucpath[k], tmp2);
			op_pos += j;
			if (j)
			    tmp2[j++] = '/';
			strcpy (&tmp2[j], tmp3);
		    }
		}
/* r  = READ only access;
 * w  = WRITE new file;
 * a  = WRITE append;
 * r+ = READ/WRITE access;
 * w+ = WRITE new file;
 * a+ = WRITE append;
 */
		j = tmp[0];
		if (j == 'r' && timeout < 0L) {
		    errno = 0;
		    while ((opnfile[k] = fopen (tmp2, tmp)) == NULL) {
			if (errno == EINTR) {
			    errno = 0;
			    continue;
			}		/* interrupt */
			if (errno == EMFILE || errno == ENFILE) {
			    close_all_globals ();
			    continue;
			}
			if (dev[k][0] != '/' && oucpath[op_pos++] != EOL) {
			    strcpy (tmp2, tmp3);
			    goto open20;
			}
			act_oucpath[k][0] = EOL;
			ierr = (errno == ENOENT ? FILERR : PROTECT);
			goto err;
		    }
		    devopen[k] = ((i == '+') ? i : j);
		    break;
		}
		if (j == 'r' || j == 'w' || j == 'a') {
		    if (timeout >= 0L) {
			test = TRUE;
			if (setjmp (sjbuf)) {
			    test = FALSE;
			    goto endopn;
			}
			SIGNAL_ACTION(SIGALRM, ontimo, NULL);
			alarm ((unsigned) (timeout < 3 ? 3 : timeout));
		    }
		    for (;;)
		    {
			errno = 0;
			if ((opnfile[k] = fopen (tmp2, tmp)) != NULL)
			    break;
			if (ierr == INRPT)
			    goto err;
			if (errno == EINTR)
			    continue;	/* interrupt */
			if (errno == EMFILE || errno == ENFILE) {
			    close_all_globals ();
			    continue;
			}
			if (timeout < 0L) {
			    if (dev[k][0] != '/' && oucpath[op_pos++] != EOL) {
				strcpy (tmp2, tmp3);
				goto open20;
			    }
			    if (errno == ENOENT)
				continue;
			    act_oucpath[k][0] = EOL;
			    ierr = PROTECT;
			    goto err;
			}
			if (timeout == 0L) {
			    test = FALSE;
			    goto endopn;
			}
			sleep (1);
			timeout--;
		    }
		    devopen[k] = ((i == '+') ? i : j);
		  endopn:;
		    alarm (0);		/* reset alarm request */
		} else {
		    ierr = ARGLIST;
		    goto err;
		}
	    }
	}
	break;

    case CLOSE:

/* no arguments: close all exept HOME */
	if (*codptr == SP || *codptr == EOL) {
	    if (standard) {
		ierr = NOSTAND;
		break;
	    }
	    j = 1;
	    while (j <= MAXDEV) {
		if (jour_flag && (j == 2)) {
		    j++;
		    continue;
		}
		if (devopen[j])
		    fclose (opnfile[j]);
		devopen[j++] = 0;
	    }
	    io = HOME;
	    break;
	}
	expr (STRING);
	j = intexpr (argptr);
	if (ierr > OK)
	    break;
	if ((j >= 0 && j <= MAXDEV && j != HOME) &&	/*ignore close on illgal units */
	    (jour_flag == 0 || (j != 2))) {	/*ignore close on protocol channel */
	    if (devopen[j])
		fclose (opnfile[j]);
	    devopen[j] = 0;
	    if (io == j)
		io = HOME;
	}
/* parse any 'device parameters', but ignore them otherwise */
	if (*codptr == ':') {
	    if (*++codptr != '(')
		expr (STRING);
	    else {
		setpiece = TRUE;	/* to avoid bracket error at end of deviceparameters */
		for (;;)
		{
		    if (*++codptr != ':')
			expr (STRING);
		    if (*codptr == ':')
			continue;
		    if (*codptr++ != ')')
			ierr = ARGER;
		    break;
		}
		setpiece = FALSE;
	    }
	}
	break;

    case HA:				/* HALT or HANG */
    case ZHALT:			/* ZHALT */
/* no arguments: HALT */
	if (*codptr == SP || *codptr == EOL || mcmnd == ZHALT) {
	    if (mcmnd == ZHALT && *codptr != SP && *codptr != EOL) {
		expr (STRING);
		i = intexpr (argptr);
		if (ierr > OK)
		    break;
	    } else {
	      halt:i = 0;
	    }
	    cleanup ();
	    if (father) {		/* advertise death to parent *//* make sure father is waiting !!! */
		if ((time (0L) - jobtime) < 120)
		    sleep (2);
		kill (father, SIGUSR1);
	    }
	    exit (i);			/* terminate mumps */
	};
/* with arguments: HANG */

    case HANG:				/* HANG */
	{
	    unsigned long int waitsec;
	    int     millisec;
#ifdef USE_GETTIMEOFDAY
	    struct timeval timebuffer;
#else
	    struct timeb timebuffer;
#endif

	    expr (STRING);
	    numlit (argptr);
	    if (ierr > OK)
		break;
	    if (argptr[0] == '-')
		break;			/* negative values without effect */
	    if (argptr[0] == '0')
		break;			/* zero without effect */
	    waitsec = 0;
	    millisec = 0;
	    i = 0;
	    for (;;)			/* get integer and fractional part */
	    {
		if ((ch = argptr[i++]) == EOL)
		    break;
		if (ch == '.') {
		    millisec = (argptr[i++] - '0') * 100;
		    if ((ch = argptr[i++]) != EOL) {
			millisec += (ch - '0') * 10;
			if ((ch = argptr[i]) != EOL) {
			    millisec += (ch - '0');
			}
		    }
		    break;
		}
		waitsec = waitsec * 10 + ch - '0';
	    }
	    if ((i = waitsec) > 2)
		i -= 2;

#ifdef USE_GETTIMEOFDAY
	    gettimeofday (&timebuffer, NULL);	/* get current time */
	    waitsec += timebuffer.tv_sec;	/* calculate target time */
	    millisec += timebuffer.tv_usec;
#else
	    ftime (&timebuffer);	/* get current time */
	    waitsec += timebuffer.time;	/* calculate target time */
	    millisec += timebuffer.millitm;
#endif

	    if (millisec >= 1000) {
		waitsec++;
		millisec -= 1000;
	    }
/* do the bulk of the waiting with sleep() */
	    while (i > 0) {
		j = time (0L);
		sleep ((unsigned) (i > 32767 ? 32767 : i));	/* sleep max. 2**15-1 sec */
		i -= time (0L) - j;	/* subtract actual sleeping time */
	    }
/* do the remainder of the waiting watching the clock */
	    for (;;)
	    {
#ifdef USE_GETTIMEOFDAY
		gettimeofday (&timebuffer, NULL);
		if (timebuffer.tv_sec > waitsec) break;
		if (timebuffer.tv_sec == waitsec &&
		    timebuffer.tv_usec >= millisec)
		    break;
#else
		ftime (&timebuffer);
		if (timebuffer.time > waitsec) break;
		if (timebuffer.time == waitsec &&
		    timebuffer.millitm >= millisec)
		    break;
#endif
	    }
	}
	break;

    case HALT:				/* HALT */
	if (*codptr == SP || *codptr == EOL)
	    goto halt;
	ierr = ARGLIST;
	break;

    case BREAK:
	if (*codptr == SP || *codptr == EOL) {
	    if (breakon == FALSE)
		break;			/* ignore BREAK */
	    if (usermode == 0) {
		ierr = BKERR;
		goto err;
	    }
	    zbflag = TRUE;
	    ierr = OK - CTRLB;
	  zb_entry:loadsw = TRUE;
#ifdef DEBUG_NEWSTACK
            printf("CHECK 08 (Stack PUSH)\r\n"); 
#endif
#ifdef NEWSTACK
            tmp_stack = (stack_level *)calloc(1,sizeof(stack_level));
            if(!tmp_stack) { ierr = STKOV; goto err; }
            tmp_stack->command  = BREAK;
#ifdef DEBUG_NEWSTACK
                if(!cmdptr) printf("CMDPTR is ZERO!\r\n");
#endif          
            tmp_stack->cmdptr   = cmdptr;
            tmp_stack->rouname  = NULL;
            tmp_stack->roucur   = roucur - rouptr;
            tmp_stack->ztrap[0] = EOL;
            tmp_stack->level    = level; level = 0;
            tmp_stack->brk      = (((ECHOON ? 1 : 0) << 1) | test) << 3 | io;

            tmp_stack->previous = stack; tmp_stack->new = NULL;
            stack = tmp_stack;
#else /* NEWSTACK */
	    if (++nstx > NESTLEVLS) {
		nstx--;
		ierr = STKOV;
		goto err;
	    }
	    nestc[nstx] = BREAK;
#ifdef DEBUG_NEWSTACK
                if(!cmdptr) printf("CMDPTR is ZERO!\r\n");
#endif          
	    nestp[nstx] = cmdptr;	/* command stack address */
	    nestn[nstx] = 0;		/*!!! save name */
	    nestr[nstx] = roucur - rouptr;	/* save roucur */
	    nestnew[nstx] = 0;
	    ztrap[nstx][0] = EOL;
	    nestlt[nstx] = level;
	    level = 0;			/* save level */
/* save BREAK information   */
	    brkstk[nstx] = (((ECHOON ? 1 : 0) << 1) | test) << 3 | io;
#endif
	    io = HOME;
	    forsw = FALSE;
	    cmdptr += stcpy (cmdptr, codptr) + 1;
	    zerr = BKERR;
	    goto restart;
	}
	expr (STRING);
	if (ierr > OK)
	    break;
	switch (intexpr (argptr)) {
	case 2:
	    DSM2err = TRUE;
	    break;			/* enable DSM V 2 error processing */
	case -2:
	    DSM2err = FALSE;
	    break;			/* enable normal error processing  */
	case 0:
	    breakon = FALSE;
	    break;			/* disable CTRL/C */
	default:
	    breakon = TRUE;
	    break;			/* enable CTRL/C  */
	}
	break;

    case VIEW:

	view_com ();

	if (repQUIT) {			/* VIEW 26: repeated QUIT action */
	    while (repQUIT-- > 0) {
#ifdef DEBUG_NEWSTACK
                printf("CHECK 09 (Stack POP)\r\n"); 
#endif
#ifdef NEWSTACK
		if (stack->command == BREAK) {
		    if (repQUIT) continue;
		    ierr = OK - CTRLB;
		    goto zgo;	
		}
		if (stack->command == FOR) {
		    stcpy (code, cmdptr = stack->cmdptr);
		    codptr = code;
		    ftyp = fortyp[--forx];
		    fvar = forvar[forx];
		    finc = forinc[forx];
		    flim = forlim[forx];
		    fi = fori[forx];
                    tmp_stack = stack; stack = stack->previous; free(tmp_stack);
		    if (repQUIT) continue;
		    if (stack && (forsw = (stack->command == FOR))) {
                       goto for_end;
                    }
		    goto next_line;
		}
		if (stack->rouname) {	/* reload routine */
		    namptr = stack->rouname;
		    if ((stack->command != XECUTE) || loadsw) {
			stcpy (rou_name, namptr); zload (rou_name);
			dosave[0] = 0;
		    }
		}
		if ((mcmnd = stack->command) == BREAK) {
		    if (repQUIT) continue; goto restore;
		}			
		if (mcmnd == DO_BLOCK) { test = stack->level; level--; }
		else level = stack->level;	/* pop level */
		roucur = stack->roucur + rouptr;
		stcpy (codptr = code, cmdptr = stack->cmdptr);

                tmp_stack = stack; stack = stack->previous; free(tmp_stack);
		if(stack) forsw = (stack->command == FOR);
                else forsw = 0;
#else /* NEWSTACK */
		if (nestc[nstx] == BREAK) {
		    if (repQUIT)
			continue;
		    ierr = OK - CTRLB;
		    goto zgo;		/*cont. single step */
		}
		if (nestc[nstx] == FOR) {
		    stcpy (code, cmdptr = nestp[nstx--]);
		    codptr = code;
		    ftyp = fortyp[--forx];
		    fvar = forvar[forx];
		    finc = forinc[forx];
		    flim = forlim[forx];
		    fi = fori[forx];
		    if (repQUIT)
			continue;
		    if ((forsw = (nestc[nstx] == FOR)))
			goto for_end;
		    goto next_line;
		}
		if (nestn[nstx]) {	/* reload routine */
		    namptr = nestn[nstx];
		    if ((nestc[nstx] != XECUTE) || loadsw) {
			stcpy (rou_name, namptr);
			zload (rou_name);
			dosave[0] = 0;
		    }
		    namptr--;
		}
		if (nestnew[nstx])
		    unnew ();		/* un-NEW variables */
/* restore old pointers */
		if ((mcmnd = nestc[nstx]) == BREAK) {
		    if (repQUIT)
			continue;
		    goto restore;
		}			/*cont. single step */
		if (mcmnd == DO_BLOCK) {
		    test = nestlt[nstx];
		    level--;
		}
/* pop $TEST */
		else
		    level = nestlt[nstx];	/* pop level */
		roucur = nestr[nstx] + rouptr;
		stcpy (codptr = code, cmdptr = nestp[nstx--]);
		forsw = (nestc[nstx] == FOR);
#endif /* NEWSTACK */
		loadsw = TRUE;

		if (mcmnd == '$') {
		    if (repQUIT) return 0;
		    ierr = NOVAL;
		}
	    }
	    repQUIT = 0;
	}
	break;

/* Z-COMMANDS */

    case ZGO:

/* ZGO with arguments: same as GOTO but with BREAK on */
	if (*codptr != EOL && *codptr != SP) {
	    mcmnd = GOTO;
	    zbflag = TRUE;
	    ierr = OK - CTRLB;
	    goto do_goto;
	}
/* argumentless ZGO resume execution after BREAK */
#ifdef NEWSTACK
        if(!stack || (stack->command != BREAK)) { ierr = LVLERR; break; }
#else
	if (nestc[nstx] != BREAK) {
	    ierr = LVLERR;
	    break;
	}
#endif
	ierr = OK;			/* stop BREAKing */
      zgo:
#ifdef DEBUG_NEWSTACK
        printf("Zgoing: (Stack POP)\r\n");
#endif
#ifdef NEWSTACK 
        if(stack) {
          if (stack->rouname) {		/* reload routine */
	    stcpy (rou_name, (namptr = stack->rouname)); zload (rou_name);
	    if (ierr > OK) break;
	  }
	  level = stack->level;
	  roucur = stack->roucur + rouptr;
	  io = stack->brk;
	  if (io & 020) DSW &= ~BIT0;
	  else DSW |= BIT0;		/* restore echo state */
	  test = (io & 010) >> 3;		/* restore $TEST */
	  if ((io &= 07) != HOME && devopen[io] == 0)
	    io = HOME;
	  stcpy (codptr = code, cmdptr = stack->cmdptr);
          tmp_stack = stack; stack = stack->previous; free(tmp_stack);
          if(stack) forsw = (stack->command == FOR);
          else forsw = 0;
        } else { printf("Panic attack in Xecline!\r\n"); }
#else /* NEW STACK */
        if (nestn[nstx]) {		/* reload routine */
	    stcpy (rou_name, (namptr = nestn[nstx]));
	    zload (rou_name);
	    if (ierr > OK)
		break;
	}
	level = nestlt[nstx];
	roucur = nestr[nstx] + rouptr;
	io = brkstk[nstx];
	if (io & 020)
	    DSW &= ~BIT0;
	else
	    DSW |= BIT0;		/* restore echo state */
	test = (io & 010) >> 3;		/* restore $TEST */
/* restore $IO; default to HOME if channel not OPEN */
	if ((io &= 07) != HOME && devopen[io] == 0)
	    io = HOME;
	stcpy (codptr = code, cmdptr = nestp[nstx--]);
	forsw = (nestc[nstx] == FOR);
#endif
	loadsw = TRUE;
	zbflag = FALSE;
	goto next0;

    case ZBREAK:

	if (*codptr == SP || *codptr == EOL) {
	    ierr = ARGLIST;
	    break;
	}
	expr (STRING);
	if (ierr > OK)
	    break;
	zbreakon = tvexpr (argptr);
	if (hardcopy == DISABLE)
	    set_zbreak (zbreakon ? STX : -1);	/* enable/disable CTRL/B */
	zbflag = FALSE;
	break;

    case ZQUIT:			/* ZQUIT */

	if (*codptr != EOL && *codptr != SP) {
	    ierr = ARGER;
	    break;
	}
	*tmp4 = EOL;
	for (;;)
	{
#ifdef DEBUG_NEWSTACK
            printf("CHECK 10\r\n"); 
#endif
#ifdef NEWSTACK
            if(stack->ztrap[0] != EOL && !DSM2err) {
		tmp4[0] = GOTO;
		tmp4[1] = SP;		/* GOTO errorhandling */
		stcpy (&tmp4[2], stack->ztrap);
		stack->ztrap[0] = EOL;
		break;
	    }
            if(!stack) {
		forx = 0;
		cmdptr = cmdstack;
		namptr = namstck;
                /* ToDo: DSM V.2 error trapping */
		break;
	    }
	    if (stack->command == FOR) {
		ftyp = fortyp[--forx];
		fvar = forvar[forx];
		finc = forinc[forx];
		flim = forlim[forx];
		fi = fori[forx];
	    } else {
		if (stack->command == DO_BLOCK) {
		    test = stack->level; level--;
		}
		else level = stack->level;	/* pop level */
            }
	    if (stack->rouname) {		/* reload routine */
	       namptr = stack->rouname;
	       stcpy (rou_name, namptr); zload (rou_name); dosave[0] = 0;
	    }
	    roucur = stack->roucur + rouptr;
	    cmdptr = stack->cmdptr; 
            if(stack->new) unnew ();	/* un-NEW variables */

            tmp_stack = stack; stack = stack->previous; 
            free(tmp_stack);
#else /* NEWSTACK */
	    if (ztrap[nstx][0] != EOL && !DSM2err) {
		tmp4[0] = GOTO;
		tmp4[1] = SP;		/* GOTO errorhandling */
		stcpy (&tmp4[2], ztrap[nstx]);
		ztrap[nstx][0] = EOL;
		break;
	    }
	    if (nstx == 0) {
		forx = 0;
		cmdptr = cmdstack;
		namptr = namstck;
		if (DSM2err && (ztrap[NESTLEVLS + 1][0] != EOL)) {	/* DSM V.2 error trapping */
		    tmp4[0] = GOTO;
		    tmp4[1] = SP;	/* GOTO errorhandling */
		    stcpy (&tmp4[2], ztrap[NESTLEVLS + 1]);
		    ztrap[NESTLEVLS + 1][0] = EOL;
		}
		break;
	    }
	    if (nestc[nstx] == FOR) {
		ftyp = fortyp[--forx];
		fvar = forvar[forx];
		finc = forinc[forx];
		flim = forlim[forx];
		fi = fori[forx];
	    } else {
		if (nestc[nstx] == DO_BLOCK) {
		    test = nestlt[nstx];
		    level--;
		}
/* pop $TEST */
		else
		    level = nestlt[nstx];	/* pop level */
	    }
	    if (nestn[nstx]) {		/* reload routine */
		namptr = nestn[nstx];
		stcpy (rou_name, namptr);
		zload (rou_name);
		dosave[0] = 0;
		namptr--;
	    }
	    roucur = nestr[nstx] + rouptr;
	    cmdptr = nestp[nstx];
	    if (nestnew[nstx])
		unnew ();		/* un-NEW variables */
	    nstx--;
#endif
	}

	forsw = FALSE;
	if (*tmp4 == EOL)
	    goto restore;
	stcpy (code, tmp4);
	codptr = code;
	goto next_cmnd;

    case ZLOAD:

	if (*codptr == EOL || *codptr == SP) {
	    stcpy (varnam, rou_name);
	} else {
	    expr (NAME);
	    if (ierr > OK)
		break;
	    codptr++;
	}
	dosave[0] = EOL;
	if (varnam[0] == EOL) {
	    varerr[0] = EOL;
	    ierr = NOPGM;
	    break;
	}				/*error */
	loadsw = TRUE;
/* a ZLOAD on the active routine always loads from disk */
	if (stcmp (varnam, rou_name) == 0) {
	    for (i = 0; i < NO_OF_RBUF; i++) {
		if (rouptr == (buff + (i * PSIZE0))) {
		    pgms[i][0] = EOL;
		    break;
		}
	    }
	}
	zload (varnam);
	if (ierr > OK)
	    break;			/* load file */
	stcpy (rou_name, varnam);
	break;

    case ZSAVE:

	if (*codptr == EOL || *codptr == SP) {
	    if (rou_name[0] == EOL) {
		varerr[0] = EOL;
		ierr = NOPGM;
		break;
	    }				/*error */
	    stcpy (varnam, rou_name);
	} else {
	    expr (NAME);
	    if (varnam[0] == '^')
		ierr = GLOBER;
	    if (varnam[0] == '$')
		ierr = INVREF;
	    if (ierr > OK)
		break;
	    stcpy (rou_name, varnam);
	    codptr++;
	}
	zsave (varnam);
	break;

    case ZREMOVE:

	{
	    char   *beg,
	           *end;

	    dosave[0] = EOL;
	    if (*codptr == SP || *codptr == EOL) {	/* no args is ZREMOVE  all */
		loadsw = TRUE;
		for (i = 0; i < NO_OF_RBUF; i++) {
		    if (rouptr == buff + (i * PSIZE0)) {
			pgms[i][0] = EOL;
			break;
		    }
		}
		rouptr = buff + (i * PSIZE0);
		rouend = rouins = rouptr;
		roucur = buff + (NO_OF_RBUF * PSIZE0 + 1);
		*(rouptr) = EOL;
		*(rouptr + 1) = EOL;
		*(rouptr + 2) = EOL;
		argptr = partition;
		rou_name[0] = EOL;
		break;
	    }
	    if (*codptr == ':')
		beg = rouptr;
	    else if (*codptr == '*') {
		beg = rouptr;
		while ((end = (beg + UNSIGN (*beg) + 2)) < rouins)
		    beg = end;
		codptr++;
	    } else {
		lineref (&beg);
		if (ierr > OK)
		    break;
	    }
	    if ((end = beg) == 0) {
		ierr = LBLUNDEF;
		break;
	    }
	    if (*codptr == ':') {	/* same as above */
		codptr++;
		if (*codptr == '*') {
		    end = rouins;
		    codptr++;
		} else if (*codptr == ',' || *codptr == SP || *codptr == EOL) {
		    end = rouend;
		} else {
		    lineref (&end);
		    if (end == 0)
			ierr = LBLUNDEF;
		    if (ierr > OK)
			break;
		    end = end + UNSIGN (*end) + 2;
		}
	    } else
		end = end + UNSIGN (*end) + 2;

	    if (beg < rouend) {		/* else there's nothing to zremove */
		if (end >= rouend) {
		    end = rouend = beg;
		} else {
		    rouins = beg;
		    while (end <= rouend)
			*beg++ = (*end++);
		    i = beg - end;
		    rouend += i;
		    if (roucur > end)
			roucur += i;
		}
		*end = EOL;
		*(end + 1) = EOL;
		for (i = 0; i < NO_OF_RBUF; i++) {
		    if (rouptr == (buff + (i * PSIZE0))) {
			ends[i] = rouend;
			break;
		    }
		}
	    }
	    break;
	}

    case ZINSERT:

	{
	    char   *beg;

	    if (*codptr == EOL || *codptr == SP) {
		ierr = ARGLIST;
		break;
	    }				/*error */
	    dosave[0] = EOL;

/* parse stringlit */
	    expr (STRING);
	    if (ierr > OK)
		break;
	    if (*codptr != ':') {
		zi (argptr, rouins);
		break;
	    }
	    stcpy (tmp, argptr);
	    codptr++;
	    lineref (&beg);
	    if (ierr > OK)
		break;			/* parse label */
	    if (beg)
		beg = beg + UNSIGN (*beg) + 2;
	    else
		beg = rouptr;
	    if (beg > rouend + 1) {
		ierr = LBLUNDEF;
		break;
	    }
/* insert stuff */
	    zi (tmp, beg);
	    break;
	}

/* PRINT is convenient -
 * but non-standard ZPRINT should be used instead */
    case 'p':
	if (standard) {
	    ierr = NOSTAND;
	    break;
	}
    case ZPRINT:

	{
	    char   *beg,
	           *end;

	    if (*codptr == SP || *codptr == EOL) {	/* no args is ZPRINT all */
		beg = rouptr;
		end = rouend;
	    } else {
		if (*codptr == ':')
		    beg = rouptr;	/* from begin */
		else if (*codptr == '*') {	/* from 'linepointer' */
		    beg = rouptr;
		    while ((end = (beg + UNSIGN (*beg) + 2)) < rouins)
			beg = end;
		    codptr++;
		} else {
		    lineref (&beg);
		    if (ierr > OK)
			break;
		}			/* line reference */
		if (beg == 0) {
		    beg = rouptr;
		    rouins = beg;
		    if (*codptr != ':')
			break;
		}
		if (*codptr == ':') {
		    codptr++;		/* to end */
		    if (*codptr == SP || *codptr == ',' || *codptr == EOL)
			end = rouend;
		    else {
			if (*codptr == '*') {
			    end = rouins;
			    codptr++;
			}
/* to 'linepointer' */
			else {
			    lineref (&end);
			    if (ierr > OK)
				break;	/* line reference */
			    end = end + UNSIGN (*end) + 2;
			}
		    }
		} else
		    end = beg + 1;
	    }
	    if (rouend < end)
		end = rouend - 1;
	    for (; beg < end; beg += UNSIGN (*beg) + 2) {
		if (crlf[io])
		    write_m ("\012\201");
		else
		    write_m ("\012\015\201");
		if ((*(beg + 1)) == EOL)
		    break;
		write_m (beg + 1);
		if (ierr > OK)
		    break;
	    }
	    rouins = beg;
	}
	if (crlf[io])
	    write_m ("\012\201");
	else
	    write_m ("\012\015\201");
	break;

    case ZWRITE:
      zwrite:;
	{
	    short   k;
	    char    w_tmp[512];

	    if (io != HOME && devopen[io] == 'r') {
		ierr = NOWRITE;
		goto err;
	    }
	    tmp3[0] = SP;
	    tmp3[1] = EOL;
	    if ((ch = (*codptr)) == '(')	/* exclusive zwrite */
		for (;;)
	    {
		codptr++;
		expr (NAME);
		if (ierr > OK)
		    goto err;
		if (varnam[0] == '^') {
		    ierr = GLOBER;
		    goto err;
		}
		i = 0;
		while (varnam[i] != EOL) {
		    if (varnam[i] == DELIM) {
			ierr = SBSCR;
			goto err;
		    }
		    i++;
		}
		if (stcat (tmp3, varnam) == 0) {
		    ierr = MXSTR;
		    goto err;
		}
		if (stcat (tmp3, " \201") == 0) {
		    ierr = MXSTR;
		    goto err;
		}
		if ((ch = *++codptr) == ')') {
		    codptr++;
		    break;
		}
		if (ch != ',') {
		    ierr = COMMAER;
		    goto err;
		}
	    } else {
		if (ch != SP && ch != EOL)
		    goto zwritep;
	    }

/* no arguments: write local symbol table. */
	    stcpy (tmp, " $\201");
	    for (;;)
	    {
		ordercnt = 1L;
		symtab (bigquery, &tmp[1], tmp2);
		if (*tmp2 == EOL || ierr == INRPT)
		    break;
		w_tmp[0] = '=';
/* subscripts: internal format different from external one */
		k = 0;
		i = 1;
		j = 0;
		while ((ch = tmp2[k++]) != EOL) {
		    if (ch == '"') {
			if (j && tmp2[k] == ch)
			    k++;
			else {
			    toggle (j);
			    continue;
			}
		    }
		    if (j == 0) {
			if (ch == '(' ||
			    ch == ',') {
			    tmp[i++] = DELIM;
			    continue;
			}
			if (ch == ')')
			    break;
		    }
		    tmp[i++] = ch;
		}
		tmp[i] = EOL;
		if (kill_ok (tmp3, tmp) == 0)
		    continue;
		write_m (tmp2);
		symtab (get_sym, &tmp[1], &w_tmp[1]);
		write_m (w_tmp);
		write_m ("\012\015\201");
	    }
	    break;

	  zwritep:

	    expr (NAME);
	    if (varnam[0] == '^')
		ierr = GLOBER;
	    if (ierr > OK)
		goto err;
	    codptr++;
	    if (varnam[0] == '$') {
		if ((varnam[1] | 0140) == 'z' &&
		    (varnam[2] | 0140) == 'f') {
		    w_tmp[0] = '$';
		    w_tmp[1] = 'Z';
		    w_tmp[2] = 'F';
		    w_tmp[3] = '(';
		    for (i = 0; i < 44; i++)
			if (zfunkey[i][0] != EOL) {
			    intstr (&w_tmp[4], i + 1);
			    stcat (w_tmp, ")=\201");
			    write_m (w_tmp);
			    write_m (zfunkey[i]);
			    write_m ("\012\015\201");
			}
		    break;
		} else
		    break;		/* do not zwrite special variables etc. other than $ZF */
	    }
	    symtab (dat, varnam, tmp2);
	    if (tmp2[0] == '0')
		break;			/* variable not defined */
/* if $D(@varnam)=10 get next entry */
	    if (tmp2[1] == '0') {
		ordercnt = 1L;
		symtab (query, varnam, tmp2);
	    } else {
		k = 0;
		i = 0;
		j = 0;
		while ((ch = varnam[k++]) != EOL) {
		    if (ch == DELIM) {
			if (j) {
			    tmp2[i++] = '"';
			    tmp2[i++] = ',';
			    tmp2[i++] = '"';
			    continue;
			}
			j++;
			tmp2[i++] = '(';
			tmp2[i++] = '"';
			continue;
		    }
		    if ((tmp2[i++] = ch) == '"')
			tmp2[i++] = ch;
		}
		if (j) {
		    tmp[i++] = '"';
		    tmp2[i++] = ')';
		}
		tmp2[i] = EOL;
	    }
	    for (;;)
	    {				/* subscripts: internal format different from external one */
		k = 0;
		i = 0;
		j = 0;
		while ((ch = tmp2[k++]) != EOL) {
		    if (ch == '"') {
			if (j && tmp2[k] == ch)
			    k++;
			else {
			    toggle (j);
			    continue;
			}
		    }
		    if (j == 0) {
			if (ch == '(' ||
			    ch == ',') {
			    tmp[i++] = DELIM;
			    continue;
			}
			if (ch == ')')
			    break;
		    }
		    tmp[i++] = ch;
		}
		tmp[i] = EOL;
		i = 0;
		while (tmp[i] == varnam[i]) {
		    if (varnam[i] == EOL)
			break;
		    i++;
		}
		if (varnam[i] != EOL)
		    break;
		if (tmp[i] != EOL && tmp[i] != DELIM)
		    break;
		symtab (get_sym, tmp, &w_tmp[1]);
		write_m (tmp2);
		w_tmp[0] = '=';
		write_m (w_tmp);
		write_m ("\012\015\201");
		ordercnt = 1L;
		symtab (query, tmp, tmp2);
		if (ierr == INRPT)
		    break;
	    }
	    break;
	}


    case ZTRAP:

	if (*codptr == SP || *codptr == EOL) {
	    ierr = ZTERR;
	    varnam[0] = EOL;
	    break;
	}
	expr (NAME);
	stcpy (varerr, varnam);
	if (ierr)
	    break;
	if (*++codptr == ':') {		/* parse postcond */
	    codptr++;
	    expr (STRING);
	    if (ierr > OK)
		goto err;
	    if (tvexpr (argptr) == FALSE)
		break;
	}
	ierr = ZTERR;
	break;

    case ZALLOCATE:

/* argumentless is not permitted */
	if (*codptr == SP || *codptr == EOL) {
	    ierr = ARGLIST;
	    break;
	}
	expr (NAME);
	if (ierr > OK)
	    goto err;
	tmp[0] = SP;
	stcpy (&tmp[1], varnam);
	stcat (tmp, "\001\201");

	timeout = (-1L);		/* no timeout */
	if (*++codptr == ':') {
	    codptr++;
	    expr (STRING);
	    timeout = intexpr (argptr);
	    if (ierr > OK)
		goto err;
	    if (timeout < 0L)
		timeout = 0L;
	}
	lock (tmp, timeout, ZALLOCATE);
	break;

    case ZDEALLOCATE:

	tmp[0] = SP;
	if (*codptr == SP || *codptr == EOL) {
	    tmp[1] = EOL;
	} else {
	    expr (NAME);
	    if (ierr > OK)
		goto err;
	    stcpy (&tmp[1], varnam);
	    codptr++;
	}
	lock (tmp, -1L, ZDEALLOCATE);	/* -1: no timeout */
	break;

/* user defined Z-COMMAND */

    case PRIVATE:
private:  /* for in-MUMPS defined commands */
	i = 0;
	j = 0;
	ch = 0;
	while ((tmp2[i] = *codptr) != EOL) {
	    if (tmp2[i] == SP && !j) {
		tmp2[i] = EOL;
		break;
	    }
	    if (tmp2[i] == '"')
		j = (!j);
	    if (!j) {
		if (tmp2[i] == '(')
		    ch++;
		if (tmp2[i] == ')')
		    ch--;
		if (!ch && tmp2[i] == ',') {	/* next argument: */
		    tmp2[i] = EOL;	/* call afterwards again */
		    i = 0;
		    while (tmp3[i] != EOL)
			i++;
		    j = i;
		    ch = 1;
		    while (ch < i)
			tmp3[j++] = tmp3[ch++];
		    tmp3[j - 1] = SP;
		    tmp3[j] = EOL;
		    codptr++;
		    j = 0;
		    ch = 0;
		    break;
		}
	    }
	    i++;
	    codptr++;
	}
	if (j || ch) {
	    ierr = INVREF;
	    goto err;
	}
	stcat (tmp3, codptr);
	stcpy (code, "d ^%\201");
	stcat (code, &tmp3[1]);
	codptr = code;
	privflag = TRUE;
	goto next_cmnd;
        /* various command that are not yet implemented */
    case ABLOCK:
    case AUNBLOCK:
    case ASSIGN:
    case ASTART:
    case ASTOP:
    case ETRIGGER:
    case ESTART:
    case ESTOP:
    case TCOMMIT:
    case TRESTART:
    case TROLLBACK:
    default:
	ierr = CMMND;
    }					/* command switch */

    if ((ch = *codptr) == EOL) {
	if (ierr) goto err;
	if (forsw) goto for_end;
	mcmnd = 0;
	goto next_line;
    }
    if (ch == SP) {
	if (ierr == OK)
	    goto next0;
	goto err;
    }
    if (ch != ',' && ierr == OK)
	ierr = SPACER;
    else if (ierr <= OK) {
	if (*++codptr != SP && *codptr != EOL)
	    goto again;
	ierr = ARGLIST;
    }
/* else goto err; */

/* error */
 err:if (ierr < 0) {
	ierr += CTRLB;
	if (ierr == OK) {
	    zbflag = TRUE;
	    goto zb_entry;
	}
    }
    if (ierr != INRPT && promflag == FALSE) {
	prompt[0] = '"';
	stcpy (&prompt[1], defprompt);
	stcat (prompt, "\"\201");
    }
    zerr = ierr;
    ierr = OK;
/*     goto restart;    */


  restart:
    if (param)
	goto restore;

    dosave[0] = EOL;
    setpiece = FALSE;
    setop = 0;
    privflag = FALSE;
    if (ierr == INRPT)
	goto err;
    if (zerr == STORE)
	symtab (kill_all, "", "");
    if (errfunlvl > 0) {
	errfunlvl--;
    } else {
	if (zerr == OK)
	    zerror[0] = EOL;		/* reset error */
	else {
#ifdef DEBUG_STACK 
            printf("Storing NESTERR\r\n");
#endif
	    nesterr = nstx;		/* save stack information at error */
	    for (i = 1; i <= nstx; i++)
		getraddress (callerr[i], i);
	    zerror[0] = '<';
	    if (etxtflag)
		stcpy (&zerror[1], errmes[zerr]);
	    else
		intstr (&zerror[1], zerr);
	    stcat (zerror, ">\201");
	    if (rou_name[0] != EOL) {
		char   *j0;
		char   *j1;
		char    tmp1[256];
#ifdef NEWSTACK
                if (stack && stack->command == XECUTE) { 
                     if(stack->rouname) { zload (stack->rouname); ierr = OK; }
                     roucur = stack->roucur + rouptr;
                }
#else /* NEWSTACK */
		if (nestc[nstx] == XECUTE) {
		    if (nestn[nstx]) {	/* reload routine */
			zload (nestn[nstx]);
			ierr = OK;
		    }
		    roucur = nestr[nstx] + rouptr;	/* restore roucur */
		}
#endif
		j0 = (rouptr - 1);
		j = 0;
		tmp1[0] = EOL;
		j0++;
		if (roucur < rouend) {
		    while (j0 < (roucur - 1)) {
			j1 = j0++;
			j++;
			if ((*j0 != TAB) && (*j0 != SP)) {
			    j = 0;
			    while ((tmp1[j] = (*(j0++))) > SP) {
				if (tmp1[j] == '(')
				    tmp1[j] = EOL;
				j++;
			    }
			    tmp1[j] = EOL;
			    j = 0;
			}
			j0 = j1;
			j0 += (UNSIGN (*j1)) + 2;
		    }
		}
		stcat (zerror, tmp1);
		if (j > 0) {
		    i = stlen (zerror);
		    zerror[i++] = '+';
		    intstr (&zerror[i], j);
		}
		stcat (zerror, "^\201");
#ifdef NEWSTACK
                if (stack && stack->command == XECUTE) {
                   if(stack->rouname) { zload(rou_name); ierr = OK; }
                   stcat(zerror, stack->rouname);
                } else stcat (zerror, rou_name);
#else
		if (nestc[nstx] == XECUTE) {
		    if (nestn[nstx]) {	/* reload routine */
			zload (rou_name);
			ierr = OK;
		    }
		    stcat (zerror, nestn[nstx]);
		} else
		    stcat (zerror, rou_name);
#endif
	    }
	    if (zerr == UNDEF ||	/* undefined: report variable name */
		zerr == SBSCR ||
		zerr == NAKED ||
		zerr == ZTERR ||
		zerr == DBDGD ||
		zerr == LBLUNDEF ||
		zerr == NOPGM) {
		int     f;		/* include erroneous reference */

		f = stlen (zerror);
		zerror[f++] = SP;
		zname (&zerror[f], varerr);
	    }				/* end varnam section */
	}
    }
    roucur = buff + (NO_OF_RBUF * PSIZE0 + 1);
    tmp4[0] = EOL;
    while (ierr != (OK - CTRLB)) {
#ifdef DEBUG_NEWSTACK
        printf("CHECK 11 [DSM2err] is [%d]\r\n",DSM2err); 
#endif
#ifdef NEWSTACK  
        /* ToDo DSM V.2 Error Handling */
        if (!stack) {
#ifdef DEBUG_NEWSTACK
           printf("[Stack Initialized]\r\n");
#endif
           forx = 0; cmdptr = cmdstack; namptr = namstck; level = 0;
           errfunlvl = 0; io = HOME;
           if (zerr == INRPT && filter) { tmp4[0] = 'h'; tmp4[1] = EOL; }
           break;
        } else {
          if(stack->ztrap[0] != EOL && !DSM2err) { /* Funky Error */
#ifdef DEBUG_NEWSTACK
            printf("Ztrap\r\n");
#endif
            tmp4[0] = GOTO; tmp4[1] = SP;
            stcpy( &tmp4[2], stack->ztrap );
            stack->ztrap[0] = EOL;
#ifdef DEBUG_NEWSTACK
            printf("Set tmp4 to [");
            for(loop=0; tmp4[loop] != EOL; loop++) printf("%c",tmp4[loop]);
            printf("]\r\n");
#endif
            break;
          }
#ifdef DEBUG_NEWSTACK
          printf("Stack OK and [command] = [%d]\r\n",stack->command);
#endif
	  if (stack->command == BREAK) break;
	  if (stack->command == FOR) {
	    ftyp = fortyp[--forx];
	    fvar = forvar[forx];
	    finc = forinc[forx];
	    flim = forlim[forx];
	    fi = fori[forx];
	  } else {
	    if (stack->command == DO_BLOCK) { test = stack->level; level--; }
            else level = stack->level;	/* pop level */
	  }
	  if (stack->rouname) {		/* 'reload' routine */
	    namptr = stack->rouname; stcpy (rou_name, namptr);
	    zload (rou_name); dosave[0] = 0;
	  }
	  roucur = stack->roucur + rouptr;
	  if (stack->new) unnew ();
	  cmdptr = stack->cmdptr;
	  if (stack->command == '$') {/* extrinsic function/variable */
	    *argptr = EOL;
	    ierr = zerr;
	    errfunlvl++;
            tmp_stack = stack; stack = stack->previous; free(tmp_stack);
	    return 0;
	  }
          tmp_stack = stack; stack = stack->previous; free(tmp_stack);
        }
#else /* NEWSTACK */
	if (ztrap[nstx][0] != EOL && !DSM2err) {	/* GOTO errorhandling */
#ifdef DEBUG_NEWSTACK
            printf("Dropped into Ztrap [");
            for(loop=0; loop<20 && ztrap[nstx][loop] != EOL; loop++) {
              printf("%c",ztrap[nstx][loop]);
            }
            printf("]\r\n"); 
#endif
	    tmp4[0] = GOTO; tmp4[1] = SP;
	    stcpy (&tmp4[2], ztrap[nstx]);
	    ztrap[nstx][0] = EOL;
#ifdef DEBUG_NEWSTACK
            printf("Set tmp4 to [");
            for(loop=0; tmp4[loop] != EOL; loop++) printf("%c",tmp4[loop]);
            printf("]\r\n");
#endif
	    break;
	}
	if (nstx == 0) {
#ifdef DEBUG_NEWSTACK
            printf("Nestx was Zero\r\n");
#endif
	    forx = 0;
	    cmdptr = cmdstack;
	    namptr = namstck;
	    level = 0;
	    errfunlvl = 0;
	    io = HOME;			/* trap to direct mode: USE 0 */
	    if (zerr == INRPT && filter) {
		tmp4[0] = 'h';
		tmp4[1] = EOL;
	    }
	    if (DSM2err && (ztrap[NESTLEVLS + 1][0] != EOL)) {	/* DSM V.2 error trapping */
#ifdef DEBUG_NEWSTACK
            printf("Ztrap 2\r\n");
#endif
		tmp4[0] = GOTO;
		tmp4[1] = SP;		/* GOTO errorhandling */
		stcpy (&tmp4[2], ztrap[NESTLEVLS + 1]);
		ztrap[NESTLEVLS + 1][0] = EOL;
	    }
	    break;
	}
#ifdef DEBUG_NEWSTACK
            printf("Nestc[nstx] is [%d]\r\n",nestc[nstx]);
#endif
	if (nestc[nstx] == BREAK) break;
	if (nestc[nstx] == FOR) {
	    ftyp = fortyp[--forx];
	    fvar = forvar[forx];
	    finc = forinc[forx];
	    flim = forlim[forx];
	    fi = fori[forx];
	} else {
	    if (nestc[nstx] == DO_BLOCK) {
		test = nestlt[nstx];
		level--;
	    }
/* pop $TEST */
	    else
		level = nestlt[nstx];	/* pop level */
	}
#ifdef DEBUG_NEWSTACK
            printf("Nestn[nstx] is [%d]\r\n",nestn[nstx]);
#endif
	if (nestn[nstx]) {		/* 'reload' routine */
	    namptr = nestn[nstx];
	    stcpy (rou_name, namptr);
	    zload (rou_name);
	    dosave[0] = 0;
	    namptr--;
	}
#ifdef DEBUG_NEWSTACK
        printf("Execcing the rest...\r\n");
#endif
	roucur = nestr[nstx] + rouptr;
	if (nestnew[nstx])
	    unnew ();			/* un-NEW variables */
	cmdptr = nestp[nstx];
	if (nestc[nstx--] == '$') {	/* extrinsic function/variable */
	    *argptr = EOL;
	    ierr = zerr;
	    errfunlvl++;
	    return 0;
	}
#endif
    }

    forsw = FALSE;
    if (tmp4[0] == EOL) {
	if (zerr == BKERR && brkaction[0] != EOL) {
	    stcpy (code, brkaction);
	    codptr = code;
	    goto next_cmnd;
	}
	DSW &= ~BIT0;			/* enable ECHO */
	write_m ("\r\n>> <\201");
	if (etxtflag)
	    write_m (&zerror[1]);
	else {
	    write_m (errmes[zerr]);
	    write_m (&zerror[zerr < 10 ? 2 : 3]);
	}
	write_m ("\r\n>> \201");
	write_m (code);
	write_m ("\r\n\201");
	write_t (codptr - code + 2);
	write_m ("^\201");
    } else {
	stcpy (code, tmp4);
	codptr = code;
	tmp4[0] = EOL;
	goto next_cmnd;
    }

  restore:
    io = HOME;
    codptr = code;
    if (param > 0) {
	j = 0;
	ch = 0;
	paramx++;
	param--;
	for (;;)
	{
	    if (m_argv[++j][0] == '-') {
		i = 0;
		while ((m_argv[j][++i] != 0) && (m_argv[j][i] != 'x')) ;
		if (m_argv[j][i] != 'x')
		    continue;
		j++;
		if (++ch < paramx)
		    continue;
		strcpy (code, m_argv[j]);
		break;
	    } else {
		if (++ch < paramx)
		    continue;
		strcpy (code, "d ");
		strcpy (&code[2], m_argv[j]);
		break;
	    }
	}
	code[strlen (code)] = EOL;
	codptr = code;
	goto next_cmnd;
    }
    if (usermode == 0) {		/* application mode: direct mode implies HALT */
	code[0] = 'H';
	code[1] = EOL;
	codptr = code;
	goto next_cmnd;
    }
    do {
	if (filter == FALSE && promflag) {
	    stcpy (code, "w \201");
	    stcpy (&code[2], prompt);
	    promflag = FALSE;
	} else {
	    read_m (code, -1L, 0, 255);	/* Not necessarily STRLEN?*/
	    promflag = TRUE;
	    if (ierr > OK)
		goto err;
#ifdef NEWSTACK
	    if (code[0] == EOL && zbflag && stack && stack->command == BREAK) {
#else
	    if (code[0] == EOL && zbflag && nestc[nstx] == BREAK) {
#endif
		ierr = OK - CTRLB;
		goto zgo;
	    }				/* single step */
	}
    } while (code[0] == EOL);
    if (promflag)
	write_m ("\r\n\201");
/* automatic ZI in direct mode: insert an entry with TAB */
    i = (-1);
    j = 0;
    ierr = OK;
    while (code[++i] != EOL) {
	if (code[i] == '"')
	    toggle (j);
	if (code[i] == TAB && j == 0) {
	    dosave[0] = EOL;
	    zi (code, rouins);
	    if (ierr)
		goto err;
	    goto restore;
	}
    }


    code[++i] = EOL;
    code[++i] = EOL;

    roucur = buff + (NO_OF_RBUF * PSIZE0 + 1);
    goto next_cmnd;

  skip_line:
    if (forsw) goto for_end;
    goto next_line;



}					/*end of xecline() */

/* End of $Source: /cvsroot-fuse/gump/FreeM/src/xecline.c,v $ */
