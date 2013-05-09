/***
 * $Source: /cvsroot-fuse/gump/FreeM/src/operator.c,v $
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
 * operators pattern, divide, multiply, add, power
 * 
 */

#include "mpsdef.h"
void    root ();
void    round ();

#define PLUS	'+'
#define MINUS	'-'
#define POINT	'.'
#define point	(POINT-ZERO)
#define ZERO	'0'
#define ONE	(ZERO+1)
#define TWO	(ZERO+2)
#define THREE	(ZERO+3)
#define FIVE	(ZERO+(NUMBASE/2))
#define NINE	(ZERO+NUMBASE-1)
#define NUMBASE	10

short int
pattern (a, b)				/* evaluates a ? b */
	char   *a;
	char   *b;

{
    short   levels;			/* depth of stack */
    register patx;			/* match stack pointer */
    short   notpatclass;		/* pattern class negation */
    char   *ptrpcd[PATDEPTH],		/* pointers to patcode */
           *position[PATDEPTH];		/* position of matching substring */
    short   mincnt[PATDEPTH],		/* minimum number of matches */
            maxcnt[PATDEPTH],		/* maximum number of matches */
            actcnt[PATDEPTH];		/* actual count of matches */
    short   Pflag,
            Pchar;			/* status in pattern alternation */
    short   altc;			/* alternation counter */
    short   altcnt[PATDEPTH];		/* gr.pat.alternation counters */

    unsigned char gpmin[PATDEPTH][PATDEPTH][255];	/* grouped pattern minimum lengthes */
    char   *gp_position[PATDEPTH][PATDEPTH];	/* grouped patt.pos.of substr */

    char   *ptrtom;			/* pointer to match code */
    char    patcode;
    int     ch;
    int     i;

    pattrnflag = Pflag = FALSE;		/* incomplete match flag */
    pattrnchar = Pchar = EOL;		/* incomplete match supplement */
    notpatclass = FALSE;		/* pattern class negation */
    patx = 0;
    while (*b != EOL) {			/* get minimum repeat count */
	mincnt[patx] = 0;
	maxcnt[patx] = 255;
	altcnt[patx] = (-1);
	if (*b != '.') {
	    ch = (*b++) - '0';
	    while (*b >= '0' && *b <= '9') {
		ch *= 10;
		ch += (*b++) - '0';
	    }
	    mincnt[patx] = ch;
	    if (*b != '.')
		maxcnt[patx] = ch;
	}
/* get maximum repeat count */
	if (*b == '.') {
	    b++;
	    if (*b >= '0' && *b <= '9') {
		ch = (*b++) - '0';
		while (*b >= '0' && *b <= '9') {
		    ch *= 10;
		    ch += (*b++) - '0';
		}
		maxcnt[patx] = ch;
	    }
	}
	if (maxcnt[patx] < mincnt[patx])
	    return '2';			/* just impossible! */
	ptrpcd[patx] = b;
	actcnt[patx] = 0;
	position[patx] = 0;
/* scan strlit, ignore it when empty */
	if (*b == '"' || *b == 'z' || (*b == '\'' && *(b + 1) == '"')) {
	    if (*(++b) == DELIM) {
		b++;
		continue;
	    }
	    while (*(++b) != DELIM) ;
	    b++;
	} else if (*b == '(') {
	    i = 1;
	    b++;
	    while ((ch = *b) != EOL) {
		b++;
		if (ch == '"') {
		    while (*(++b) != DELIM) ;
		}
		if (ch == '(') {
		    i++;
		    continue;
		}
		if (ch == ')') {
		    i--;
		    if (i < 1)
			break;
		}
	    }
	} else
	    while (*(++b) >= 'A') ;
	if (++patx >= (PATDEPTH - 1))
	    return '3';			/* stack overflow */
    }
    levels = patx;
    if (*(b - 1) == 'e' && mincnt[levels - 1] == 0 && maxcnt[levels - 1] == 255)
	*(b - 1) = '~';			/* frequent special case: last pattern is '.E' */
    mincnt[levels] = maxcnt[levels] = 1;	/* sentinel, does never match */
    actcnt[levels] = 0;
    ptrpcd[levels] = b;			/* (*b==EOL) */
    patx = 0;
    while (patx <= levels) {
	while (actcnt[patx] < mincnt[patx]) {
	    actcnt[patx]++;
	    if (*a == EOL) {
		pattrnflag = TRUE;	/* incomplete match flag */
		if (patx >= levels) {
		    pattrnchar = EOL;
		    return '1';
		}
		if (patx > 0) {
		    if (actcnt[patx - 1] != maxcnt[patx - 1])
			return '0';
/* after alternation we are not sure about */
/* that supplement character               */
		    if (*(ptrpcd[patx - 1]) == '(') {
			pattrnchar = EOL;
			return '0';
		    }
		}
		if (*(ptrpcd[patx]) == '"')
		    pattrnchar = *(ptrpcd[patx] + 1);
		return '0';
	    }
	    for (;;)
	    {
/***begin section: does that char match current pattern code ***/
		ptrtom = ptrpcd[patx];
		ch = (*a);
		for (;;)
		{
		    patcode = (*ptrtom++);
		    if ((notpatclass = (patcode == '\'')))
			patcode = (*ptrtom++);
		    switch (patcode) {	/* we live in an ASCII/ISO world !! */
		    case 'c':
			if (((ch < SP && ch >= NUL)
			     || ch == DEL) != notpatclass)
			    goto match;
			break;
		    case 'n':
			if ((ch <= '9' && ch >= '0') != notpatclass)
			    goto match;
			break;
		    case 'p':
			if (((ch >= SP && ch <= '/') ||
			     (ch >= ':' && ch <= '@') ||
			     (ch >= '[' && ch <= '`') ||
			     (ch >= '{' && ch <= '~') ||
			     (ch == '\200')) != notpatclass)
			    goto match;
			break;
		    case 'a':
			if (((ch >= 'A' && ch <= 'Z') ||
			     (ch >= 'a' && ch <= 'z')) != notpatclass)
			    goto match;
			break;
		    case 'l':
			if ((ch >= 'a' && ch <= 'z') != notpatclass)
			    goto match;
			break;
		    case 'u':
			if ((ch >= 'A' && ch <= 'Z') != notpatclass)
			    goto match;
			break;
		    case 'e':
			if (!notpatclass)
			    goto match;
			break;
		    case '"':
			i = 0;
			while (a[i++] == (*ptrtom++)) ;
			if ((*--ptrtom) == DELIM) {
			    if (notpatclass)
				goto nomatch;
			    b = ptrpcd[patx] + 1;
			    while (*b++ != DELIM)
				a++;
			    goto match0;
			}
			if (notpatclass) {
			    i--;
			    while (*ptrtom++ != DELIM) {
				if (a[i++] == EOL)
				    goto nomatch;
			    }
			    b = ptrpcd[patx] + 2;
			    while (*b++ != DELIM)
				a++;
			    goto match0;
			}
			if (a[i - 1] == EOL) {
			    pattrnflag = TRUE;
			    pattrnchar = *ptrtom;
			}
			goto nomatch;

		    case '~':{
			    pattrnchar = EOL;	/* '.E' as last pat_atom */
			    pattrnflag = TRUE;
			    return '1';
			}
/* grouped pattern match */
		    case '(':{
			    char    aa[256];
			    char    bb[256];
			    int     i1,
			            min,
			            max;
			    short   pflag;
			    short   pchar;

			    if (Pflag) {
				pflag = Pflag;
				pchar = Pchar;
			    } else {
				pflag = FALSE;
				pchar = EOL;
			    }
			    if (altcnt[patx] < 0) {
				for (altc = 0; altc < PATDEPTH; altc++)
				    gpmin[patx][altc][1] = 0;
			    }
			    altcnt[patx] = 0;
			  alternation:;
			    i = 0;
			    i1 = 1;
			    while (i1) {
				bb[i] = *ptrtom++;
				if (bb[i] == '"') {
				    while ((bb[++i] = (*ptrtom++)) != DELIM) ;
				}
				if (bb[i] == '(')
				    i1++;
				if (bb[i] == ')')
				    i1--;
				if (bb[i] == ',' && i1 == 1)
				    i1--;
				i++;
			    }
			    bb[--i] = EOL;
			    pminmax (bb, &min, &max);

			    if ((i1 = gpmin[patx][altcnt[patx]][actcnt[patx]]) < min)
				gpmin[patx][altcnt[patx]][actcnt[patx]] = i1 = min;
			    gpmin[patx][altcnt[patx]][actcnt[patx] + 1] = 0;
/* too much charaters to get a match! */
			    if (i1 > max) {
				if (*(ptrtom - 1) == ',') {
				    altcnt[patx]++;
				    goto alternation;
				}
				pattrnflag = pflag;
				pattrnchar = pchar;
				goto nomatch;
			    }
/* if number of chars too small, try anyway */
/* to get info for "incomplete" match        */
			    for (i = 0; i < i1; i++)
				if ((aa[i] = a[i]) == EOL)
				    break;
			    gp_position[patx][actcnt[patx]] = a;

			    for (;;)
			    {
				aa[i] = EOL;
				i1 = pattern (aa, bb);
				if (i1 == '1') {
				    gpmin[patx][altcnt[patx]][actcnt[patx]] = i;
				    a += i;
				    goto match0;
				}
				if (i1 != '0')
				    return i1;
				if (pattrnflag) {
				    if (pflag == FALSE)
					pchar = pattrnchar;
				    else if (pchar != pattrnchar)
					pchar = EOL;
				    pflag = TRUE;
				}
				if (!pattrnflag) {
				    if (*(ptrtom - 1) == ',') {
					altcnt[patx]++;
					goto alternation;
				    }
				    pattrnflag = pflag;
				    pattrnchar = pchar;
				    goto nomatch;
				}
				if (a[i] == EOL) {
				    Pflag = pflag;
				    Pchar = pchar;
				    if (*(ptrtom - 1) == ',') {
					altcnt[patx]++;
					goto alternation;
				    }
				    pattrnflag = pflag;
				    pattrnchar = pchar;
				    return '0';
				}
				aa[i] = a[i];
				i++;
			    }
			}
/* match one of listed characters ?1Z"string" */
		    case 'z':
			for (;;)
			{
			    if ((*++ptrtom) == DELIM) {
				if (notpatclass)
				    goto match;
				goto nomatch;
			    }
			    if (ch != *ptrtom) {
				if (*(ptrtom + 1) == '.' && *(ptrtom + 2) == '.') {
				    if (ch < *ptrtom ||
					(ch > *(ptrtom + 3) && *(ptrtom + 3) != DELIM)) {
					ptrtom += 2;
					continue;
				    }
				} else
				    continue;
			    }
			    while (*++ptrtom != DELIM) ;
			    if (notpatclass)
				goto nomatch;
			    goto match;
			}

/* loadable matches */
		    case 'C':
			i = 0;
			while (zmc[i] != EOL) {
			    if (zmc[i] == ch && !notpatclass)
				goto match;
			    i++;
			}
			if (notpatclass)
			    goto match;
			break;
		    case 'N':
			i = 0;
			while (zmn[i] != EOL) {
			    if (zmn[i] == ch && !notpatclass)
				goto match;
			    i++;
			}
			if (notpatclass)
			    goto match;
			break;
		    case 'P':
			i = 0;
			while (zmp[i] != EOL) {
			    if (zmp[i] == ch && !notpatclass)
				goto match;
			    i++;
			}
			if (notpatclass)
			    goto match;
			break;
		    case 'A':
			i = 0;
			while (zmu[i] != EOL) {
			    if (zmu[i] == ch && !notpatclass)
				goto match;
			    i++;
			}
		    case 'L':
			i = 0;
			while (zml[i] != EOL) {
			    if (zml[i] == ch && !notpatclass)
				goto match;
			    i++;
			}
			if (notpatclass)
			    goto match;
			break;
		    case 'U':
			i = 0;
			while (zmu[i] != EOL) {
			    if (zmu[i] == ch && !notpatclass)
				goto match;
			    i++;
			}
			if (notpatclass)
			    goto match;
			break;
		    default:
			goto nomatch;
		    }			/* end_switch */
		}			/* end repeat */
/*** end section: does that char match current pattern atom ***/
	      nomatch:;
		if (patcode == '(') {
		    for (altc = 0; altc <= altcnt[patx]; altc++)
			gpmin[patx][altc][actcnt[patx]] = 0;
		    if (--actcnt[patx] > 0) {
			for (altc = 0; altc <= altcnt[patx]; altc++)
			    gpmin[patx][altc][actcnt[patx]]++;
			a = gp_position[patx][actcnt[patx]];	/* try previous patterns again */
			continue;
		    }
		}
		do {
		    actcnt[patx] = 0;
		    if (--patx < 0)
			return '0';	/* stack exhausted */
		    if (*(ptrpcd[patx]) == '(') {
			if (actcnt[patx] >= maxcnt[patx]) {
			    ++actcnt[patx];
			    patcode = '(';
			    goto nomatch;
			}
		    }
		} while (++actcnt[patx] > maxcnt[patx]);
		a = position[patx];	/* try previous patterns again */

	    }				/* end repeat */
	  match:;
	    a++;
	  match0:;
	}
	position[patx++] = a;		/* pos after last match */
    }
    return '0';
}					/* end of pattern */
/******************************************************************************/
void
pminmax (str, min, max)			/* auxiliary function for grouped pattern match */

/* determines                                   */
	char   *str;			/* of a pattern 'str'                           */
	int    *min,
	       *max;			/* the minimum and maximum possible length      */

{
    int     mininc,
            maxinc,
            i,
            ch;

    *min = 0;
    *max = 0;
    mininc = 0;
    maxinc = 0;
    i = 0;
    ch = 0;
    while (str[i] != EOL) {
	if (str[i] != '.') {		/* scan minimum repeat count */
	    ch = (str[i++]) - '0';
	    while (str[i] >= '0' && str[i] <= '9') {
		ch *= 10;
		ch += (str[i++]) - '0';
	    }
	    mininc = ch;
	    maxinc = ch;
	} else {
	    mininc = 0;
	    maxinc = 255;
	}
	if (str[i] == '.') {		/* scan maximum repeat count */
	    i++;
	    if (str[i] >= '0' && str[i] <= '9') {
		ch = (str[i++]) - '0';
		while (str[i] >= '0' && str[i] <= '9') {
		    ch *= 10;
		    ch += (str[i]++) - '0';
		}
	    } else
		ch = 255;
	    maxinc = ch;
	}
/* skip pattern codes */
	if (str[i] == '"') {
	    int     cnt;

	    cnt = 0;
	    while (str[++i] != DELIM)
		cnt++;
	    mininc = mininc * cnt;
	    maxinc = maxinc * cnt;
	}
	if (str[i] == 'z' || str[i] == '"') {
	    while (str[++i] != DELIM) ;
	    i++;
	} else if (str[i] == '(') {
	    char    tmp[256];
	    char   *tcur;
	    int     tmin,
	            tmax,
	            Tmin,
	            Tmax,
	            i1;

	    tmin = 255;
	    tmax = 0;
	  alternation:;
	    i1 = 1;
	    tcur = tmp;
	    while (i1) {
		ch = str[++i];
		*tcur++ = ch;
		if (ch == '"')
		    while ((*tcur++ = str[++i]) != DELIM) ;
		if (ch == '(')
		    i1++;
		if (ch == ')')
		    i1--;
		if (ch == ',' && i1 == 1)
		    i1--;
	    }
	    *(--tcur) = EOL;
	    pminmax (tmp, &Tmin, &Tmax);
	    if (Tmin < tmin)
		tmin = Tmin;
	    if (Tmax > tmax)
		tmax = Tmax;
	    if (str[i] == ',')
		goto alternation;
	    i++;
	    mininc *= tmin;
	    maxinc *= tmax;
	} else
	    while (str[++i] >= 'A') ;
	*min += mininc;
	*max += maxinc;
    }
    if (*max > 255)
	*max = 255;
    return;
}					/* end pminmax() */
/******************************************************************************/
void
add (a, b)				/* string arithmetic a+=b; */
	char   *a,
	       *b;

{
    if (b[0] == ZERO)
	return;
    if (a[0] == ZERO) {
	stcpy (a, b);
	return;
    } {
	long   dpa,			/* decimal point of 'a' */
	        dpb,			/* decimal point of 'b' */
	        lena,			/* length of 'a' */
	        lenb;			/* length of 'b' */
	char    mi;			/* minus flag */
	short   sign;			/* sign flag  if a<0<b sign=-1; */

/*            if a>0>b sign=1;  */
/*            else     sign=0;  */

	register i;
	register ch;
	register j;
	register carry;

/* look at the signs */
	mi = 0;
	sign = 0;
	if (a[0] == b[0] && a[0] == MINUS) {
	    mi++;
	    a[0] = b[0] = ZERO;
	} else if (a[0] == MINUS) {
	    sign--;
	    a[0] = NINE;
	    i = 0;
	    while ((ch = a[++i]) != EOL)
		if (ch != POINT)
		    a[i] = ZERO + NINE - ch;
	    a[--i]++;
	} else if (b[0] == MINUS) {
	    sign++;
	    b[0] = NINE;
	    i = 0;
	    while ((ch = b[++i]) != EOL)
		if (ch != POINT)
		    b[i] = ZERO + NINE - ch;
	    b[--i]++;
	}
/* search decimal points and length */
	dpa = dpb = (-1);
	i = 0;
	while (a[i] != EOL) {
	    if (a[i] == POINT)
		dpa = i;
	    i++;
	}
	lena = i;
	if (dpa < 0)
	    dpa = i;
      again:;
	i = 0;
	while (b[i] != EOL) {
	    if (b[i] == POINT)
		dpb = i;
	    i++;
	}
	lenb = i;
	if (dpb < 0)
	    dpb = i;
	if (i == 1) {
	    if (b[0] == ONE && sign == 0 && dpa > 0) {	/* frequent special case: add 1 */
		i = dpa - 1;
		while (++a[i] > NINE) {
		    a[i--] = ZERO;
		    if (i < 0) {
			i = lena;
			while (i >= 0) {
			    a[i + 1] = a[i];
			    i--;
			}
			a[0] = ONE;
			return;
		    }
		}
		return;
	    }
	}
/* copy additional trailing digits from b to a */
	if (lenb - dpb > lena - dpa) {
	    j = dpa - dpb;
	    if (lenb + j > STRLEN) {	/* round off that monster ! */
		i = STRLEN - j;
		if (b[i] < FIVE) {
		    b[i] = EOL;
		    lenb--;
		    while (b[--i] == ZERO) {
			b[i] = EOL;
			lenb--;
		    }
		} else {
		    for (;;)
		    {
			if (i >= dpb) {
			    b[i] = EOL;
			    lenb--;
			} else
			    b[i] = ZERO;
			if (--i < 0) {
			    for (i = lenb; i >= 0; i--)
				b[i + 1] = b[i];
			    b[0] = ONE;
			    dpb = ++lenb;
			    break;
			}
			if ((ch = b[i]) == POINT) {
			    dpb = i;
			    continue;
			}
			if (ch < NINE && ch >= ZERO) {
			    b[i] = ++ch;
			    break;
			}
		    }
		}
		goto again;		/* look what's left from b */
	    }
	    lenb = i = lena - dpa + dpb;
	    j = lena;
	    while ((a[j++] = b[i++]) != EOL) ;
	    lena = (--j);
	    b[lenb] = EOL;
	}
/* $justify a or b */
	i = dpa - dpb;
	if (i < 0) {
	    j = lena;
	    if ((i = (lena -= i)) > (STRLEN - 2) /*was 253*/) {
		ierr = MXSTR;
		return;
	    }
	    ch = (sign >= 0 ? ZERO : NINE);
	    while (j >= 0)
		a[i--] = a[j--];
	    while (i >= 0)
		a[i--] = ch;
	    dpa = dpb;
	} else if (i > 0) {
	    j = lenb;
	    if ((lenb = (i += lenb)) > (STRLEN - 2)/*was 253*/) {
		ierr = MXSTR;
		return;
	    }
	    ch = (sign <= 0 ? ZERO : NINE);
	    while (j >= 0)
		b[i--] = b[j--];
	    while (i >= 0)
		b[i--] = ch;
	    dpb = dpa;
	}
/* now add */
	carry = 0;
	for (i = lenb - 1; i >= 0; i--) {
	    if ((ch = a[i]) == POINT)
		continue;
	    ch += b[i] - ZERO + carry;
	    if ((carry = (ch > NINE)))
		ch -= NUMBASE;
	    a[i] = ch;
	}
	while (a[lena] != EOL)
	    lena++;
	if (carry) {
	    if ((i = (++lena)) > (STRLEN - 2)/*was 253*/) {
		ierr = MXSTR;
		return;
	    }
	    while (i > 0) {
		a[i] = a[i - 1];
		i--;
	    }
	    a[0] = ONE;
	}
	if (sign) {
	    if (a[0] == ONE) {
		a[0] = ZERO;
	    } else {
		i = 0;
		carry = 0;
		while ((ch = a[++i]) != EOL)
		    if (ch != POINT)
			a[i] = ZERO + NINE - ch;
		while (--i > 0) {
		    if (a[i] != POINT) {
			if (++a[i] <= NINE)
			    break;
			a[i] = ZERO;
		    }
		}
		mi = 1;
		a[0] = ZERO;
	    }
	    while (a[mi] == ZERO) {
		stcpy (&a[mi], &a[mi + 1]);
		dpa--;
	    }
	    if (dpa < 0)
		dpa = 0;
	}
/* remove trailing zeroes */
	i = dpa;
	while (a[i] != EOL)
	    i++;
	if (--i > dpa) {
	    while (a[i] == ZERO)
		a[i--] = EOL;
	}
/* remove trailing point */
	if (a[i] == POINT)
	    a[i] = EOL;
	if (mi) {
	    if (a[0] != ZERO) {
		i = 0;
		while (a[i++] != EOL) ;
		while (i > 0) {
		    a[i] = a[i - 1];
		    i--;
		}
	    }
	    a[0] = MINUS;
	}
	if (a[mi] == EOL) {
	    a[0] = ZERO;
	    a[1] = EOL;
	}
	return;
    }
}
/******************************************************************************/
void
mul (a, b)				/* string arithmetic a=a*b */
	char   *a,
	       *b;
{
    char    c[2*(STRLEN+1) /*was 512*/];
    short   alen,
            blen,
            clen,
            mi,
            tmpx;
    register acur;
    register bcur;
    register ccur;
    register carry;

    if (ierr > OK)
	return;				/* avoid nonsense in recursion */
/* if zero or one there's not much to do */
    if (b[1] == EOL) {
	if (b[0] == ZERO) {
	    a[0] = ZERO;
	    a[1] = EOL;
	    return;
	}
	if (b[0] <= ONE)
	    return;
	if (b[0] == TWO) {
	  multwo:acur = 0;
	    while (a[++acur] != EOL) ;
	    mi = (a[acur - 1] == FIVE);
	    carry = 0;
	    ccur = acur;
	    while (acur >= 0) {
		if ((bcur = a[--acur]) < ZERO)
		    continue;
		bcur = bcur * 2 - ZERO + carry;
		carry = 0;
		if (bcur > NINE) {
		    carry = 1;
		    bcur -= NUMBASE;
		}
		a[acur] = bcur;
	    }
	    if (carry) {
		acur = ccur;
		if (acur > (STRLEN - 1)/*was 254*/) {
		    ierr = MXSTR;
		    return;
		}
		while (acur >= 0) {
		    a[acur + 1] = a[acur];
		    acur--;
		}
		a[a[0] == MINUS] = ONE;
	    }
	    if (mi) {
		if (carry)
		    ccur++;
		acur = 0;
		while (acur < ccur)
		    if (a[acur++] == POINT) {
			a[--ccur] = EOL;
			if (acur == ccur)
			    a[--ccur] = EOL;
			return;
		    }
	    }
	    return;
	}
    }
    if (a[1] == EOL) {
	if (a[0] == ZERO) {
	    return;
	}
	if (a[0] <= ONE) {
	    stcpy (a, b);
	    return;
	}
	if (a[0] == TWO) {
	    stcpy (a, b);
	    goto multwo;
	}
    }
/* get length of strings and convert ASCII to decimal */
/* have a look at the signs */
    if ((mi = (a[0] == MINUS))) {
	a[0] = ZERO;
    }
    if (b[0] == MINUS) {
	b[0] = ZERO;
	toggle (mi);
    }
    carry = 0;
    alen = 0;
    while (a[alen] != EOL) {
	a[alen] -= ZERO;
	if (a[alen++] == point)
	    carry = alen;
    }
/* append a point on the right side if there was none */
    if (--carry < 0) {
	carry = alen;
	a[alen++] = point;
	a[alen] = 0;
    }
    ccur = 0;
    blen = 0;
    while (b[blen] != EOL) {
	b[blen] -= ZERO;
	if (b[blen++] == point)
	    ccur = blen;
    }
    if (--ccur < 0) {
	ccur = blen;
	b[blen++] = point;
	b[blen] = 0;
    }
    carry += ccur;
    if (carry > (STRLEN - 3) /*was 252*/) {
	a[0] = EOL;
	ierr = MXSTR;
	return;
    }
    ccur = clen = alen + blen;
/* init c to zero */
    while (ccur >= 0)
	c[ccur--] = 0;
    c[carry] = point;

    bcur = blen;
    clen = alen + blen - 1;
    carry = 0;
    while (bcur > 0) {
	if (b[--bcur] == point) {
	    continue;
	}
	if (c[clen] == point)
	    clen--;
	acur = alen;
	ccur = clen--;
	while (acur > 0) {
	    if (a[--acur] == point)
		continue;
	    if (c[--ccur] == point)
		--ccur;
	    tmpx = a[acur] * b[bcur] + c[ccur] + carry;
	    carry = tmpx / NUMBASE;
	    c[ccur] = tmpx % NUMBASE;
	}
	while (carry) {
	    if (c[--ccur] == point)
		ccur--;
	    if ((c[ccur] += carry) >= NUMBASE) {
		c[ccur] -= NUMBASE;
		carry = 1;
	    } else
		carry = 0;
	}
    }
/* copy result to a and convert it */
    a[ccur = clen = acur = (alen += blen)] = EOL;
    while (--ccur >= 0) {
	if (c[ccur] < NUMBASE)
	    a[ccur] = c[ccur] + ZERO;
	else
	    a[alen = ccur] = POINT;
    }
/* oversize string */
    if (acur > STRLEN) {
	if (a[acur = STRLEN] >= FIVE) {
	    int     l1;

	    l1 = STRLEN;
	    if (a[l1] >= FIVE) {
		for (;;)
		{
		    if (a[--l1] == POINT)
			l1--;
		    if (l1 < (a[0] == MINUS)) {
			for (l1 = STRLEN; l1 > 0; l1--)
			    a[l1] = a[l1 - 1];
			a[a[0] == MINUS] = ONE;
			break;
		    }
		    if ((++a[l1]) == (NINE + 1))
			a[l1] = ZERO;
		    else
			break;
		}
	    }
	}
	a[acur] = EOL;
    }
/* remove trailing zeroes */
    if (acur >= alen) {
	while (a[--acur] == ZERO)
	    a[acur] = EOL;
    }
/* remove trailing point */
    if (a[acur] == POINT)
	a[acur] = EOL;
/* remove leading zeroes */
    while (a[mi] == ZERO) {
	acur = mi;
	while ((a[acur] = a[acur + 1]) != EOL)
	    acur++;
    }
    if (a[0] == EOL) {
	a[0] = ZERO;
	a[1] = EOL;
	mi = 0;
    }
    if (mi) {
	if (a[0] != ZERO) {
	    acur = clen;
	    while (acur > 0) {
		a[acur] = a[acur - 1];
		acur--;
	    }
	}
	a[0] = MINUS;
    }
    return;
}
/******************************************************************************
 * for a detailed description of the method for the divisions see             *
 * donald e.knuth 'the art of computer programming' vol.2 p.257               *
 ******************************************************************************/
void
div (uu, v, typ)			/* divide string arithmetic */
	char   *uu,			/* dividend and result */
	       *v;			/* divisor */
	short   typ;			/* type: '/' or '\' or '#' */

{
    char    q[STRLEN + 2 /*was 257*/];	/* quotient */
    char    u[2*(STRLEN + 1)/*was 512*/];/* intermediate result */
    char    vv[STRLEN +1 /*was 256*/];
    short   d,
            d1,
            k1,
            m,
            ulen,
            vlen,
            dpu,
            dpv,
            guess,
            mi,
            plus,
            v1;
    register long int i;
    register int j;
    register int k;
    register int carry;

    if (ierr > OK)
	return;				/* avoid nonsense in recursion */
    if (uu[0] == ZERO)
	return;
#ifdef NEVER
/* should be faster on DIV 2, but causes some error
 * in connection with SQRT */
    if (v[1] == EOL && typ == '/') {
	if (v[0] == ONE)
	    return;
	if (v[0] == TWO) {
	    carry = 0;
	    k = (-1);
	    k1 = (-1);
	    while ((i = uu[++k]) != EOL) {
		if (i < ZERO) {
		    if (i == POINT) {
			k1 = k;
			if (k + zprecise < STRLEN)
			    uu[k + zprecise] = EOL;
		    }
		    continue;
		}
		if (i == EOL)
		    break;
		if (i & 01)
		    j = NUMBASE;
		i = (i + ZERO + carry) / 2;
		carry = j;
		j = 0;
		uu[k] = i;
	    }
	    j = (uu[0] == MINUS);
	    if (uu[j] == ZERO) {
		while (j < k) {
		    uu[j] = uu[j + 1];
		    j++;
		}
		k--;
	    }
	    if (carry && k < (STRLEN - 2) /*was 253*/) {
		if (k1 < 0) {
		    k1 = k;
		    uu[k++] = POINT;
		}
		uu[k++] = FIVE;
		uu[k] = EOL;
	    }
	    return;
	}
    }
#endif /* NEVER */
/* look at the signs */
    stcpy (u, uu);
    mi = 0;
    plus = 0;
    if (typ != '#') {
	if (u[0] == MINUS) {
	    u[0] = ZERO;
	    mi = 1;
	}
	if (v[0] == MINUS) {
	    v[0] = ZERO;
	    toggle (mi);
	}
    } else {
	stcpy (vv, v);
	if (u[0] == MINUS) {
	    u[0] = ZERO;
	    plus = 1;
	}
	if (v[0] == MINUS) {
	    v[0] = ZERO;
	    mi = 1;
	    toggle (plus);
	}
    }
/* convert from ASCII to 'number' */
    i = 0;
    dpv = (-1);
    k = 0;
    while ((j = v[i]) != EOL) {
	j -= ZERO;
	if (j == point)
	    dpv = i;
	if (j == 0)
	    k++;
	v[i++] = j;
    }

    v[vlen = i] = 0;
    v[i + 1] = 0;
    v[i + 2] = 0;
    if (v[0] != 0) {
	while (i >= 0) {
	    v[i + 1] = v[i];
	    i--;
	}
	v[0] = 0;
	dpv++;
    } else {
	vlen--;
    }
    d1 = 0;

    i = 0;
    dpu = (-1);
    while (u[i] != EOL) {
	u[i] -= ZERO;
	if (u[i] == point)
	    dpu = i;
	i++;
    }
    if (dpu < 0) {
	u[dpu = i++] = point;
    }
/*      u[ulen=i]=0; u[i+1]=0; u[i+2]=0;        */
    ulen = i;
    while (i < 512)
	u[i++] = 0;
    i = ulen;				/* somehow that's necessary - sometimes I check why */
    if (u[0] != 0) {
	while (i >= 0) {
	    u[i + 1] = u[i];
	    i--;
	}
	u[0] = 0;
	dpu++;
    } else {
	ulen--;
    }
    if ((vlen + zprecise) > STRLEN && (dpv + zprecise) < vlen)
	vlen -= zprecise;

    if (dpv > 0) {			/* make v an integer *//* shift v */
	d1 = vlen - dpv;
	for (i = dpv; i < vlen; i++)
	    v[i] = v[i + 1];
	vlen--;
/* remove leading zeroes */
	while (v[1] == 0) {
	    for (i = 1; i <= vlen; i++)
		v[i] = v[i + 1];
	    vlen--;
	}
	v[vlen + 1] = 0;
	v[vlen + 2] = 0;
/* shift u */
	i = dpu;
	for (j = 0; j < d1; j++) {
	    if (i >= ulen) {
		u[i + 1] = 0;
		ulen++;
	    }
	    u[i] = u[i + 1];
	    i++;
	}
	u[i] = point;
	dpu = i;
    }
    d = dpu + 1 - ulen;
    if (dpv > dpu)
	d += dpv - dpu;
    if (typ == '/')
	d += zprecise;
    if ((d + ulen) > STRLEN) {
	u[0] = EOL;
	ierr = MXSTR;
	return;
    }
    while (d > 0) {
	u[++ulen] = 0;
	d--;
    }
/* normalize */
    if ((d = NUMBASE / (v[1] + 1)) > 1) {
	i = ulen;
	carry = 0;
	while (i > 0) {
	    if (u[i] != point) {
		carry += u[i] * d;
		u[i] = carry % NUMBASE;
		carry = carry / NUMBASE;
	    }
	    i--;
	}
	u[0] = carry;
	i = vlen;
	carry = 0;
	while (i > 0) {
	    carry += v[i] * d;
	    v[i] = carry % NUMBASE;
	    carry = carry / NUMBASE;
	    i--;
	}
	v[0] = carry;
    }
/* initialize */
    j = 0;
    m = ulen - vlen + 1;
    if (m <= dpu)
	m = dpu + 1;
    for (i = 0; i <= m; q[i++] = ZERO) ;
    if (typ == '#') {
	m = dpu - vlen;
    }
    v1 = v[1];

    while (j < m) {
	if (u[j] != point) {		/* calculate guess */
	    if ((k = u[j] * NUMBASE + (u[j + 1] == point ? u[j + 2] : u[j + 1])) == 0) {
		j++;
		continue;
	    }
	    k1 = (u[j + 1] == point || u[j + 2] == point ? u[j + 3] : u[j + 2]);
	    guess = (u[j] == v1 ? (NUMBASE - 1) : k / v1);
	    if (v[2] * guess > (k - guess * v1) * NUMBASE + k1) {
		guess--;
		if (v[2] * guess > (k - guess * v1) * NUMBASE + k1)
		    guess--;
	    }
/* multiply and subtract */
	    i = vlen;
	    carry = 0;
	    k = j + i;
	    if (j < dpu && k >= dpu)
		k++;
	    while (k >= 0) {
		if (u[k] == point)
		    k--;
		if (i >= 0) {
		    u[k] -= v[i--] * guess + carry;
		} else {
		    if (carry == 0)
			break;
		    u[k] -= carry;
		}
		carry = 0;
		while (u[k] < 0) {
		    u[k] += NUMBASE;
		    carry++;
		}
		k--;
	    }
/* test remainder / add back */
	    if (carry) {
		guess--;
		i = vlen;
		carry = 0;
		k = j + i;
		if (j < dpu && k >= dpu)
		    k++;
		while (k >= 0) {
		    if (u[k] == point)
			k--;
		    if (i >= 0) {
			u[k] += v[i--] + carry;
		    } else {
			if (carry == 0)
			    break;
			u[k] += carry;
		    }
		    carry = u[k] / NUMBASE;
		    u[k] = u[k] % NUMBASE;
		    k--;
		}
	    }
	    q[j++] = guess + ZERO;
	    u[0] = 0;
	} else {
	    q[j++] = POINT;
	}
    }
/* unnormalize */
    if (typ != '#') {
	i = 0;
	while (i <= m) {
	    if ((u[i] = q[i]) == POINT)
		dpv = i;
	    i++;
	}
	k = vlen;
	k1 = dpv;
	while (k-- > 0) {
	    while (k1 <= 0) {
		for (i = (++m); i > 0; i--)
		    u[i] = u[i - 1];
		k1++;
		u[0] = ZERO;
	    }
	    u[k1] = u[k1 - 1];
	    u[--k1] = POINT;
	    dpv = k1;
	}
	u[m] = EOL;
/* rounding */

	if (typ != '/')
	    u[dpv + 1] = EOL;
	else {
	    k = dpv + zprecise;
	    k1 = u[k + 1] >= FIVE;
	    u[k + 1] = EOL;
	    if (k1) {
		do {
		    if (u[k] != POINT) {
			if ((carry = (u[k] == NINE)))
			    u[k] = ZERO;
			else
			    u[k]++;
		    }
		    k--;
		} while (carry);
	    }
	}
    } else {				/* return the remainder */
	carry = 0;
	if (d > 1) {
	    for (i = 0; i <= ulen; i++) {
		if (u[i] == point) {
		    u[i] = POINT;
		    dpu = i;
		} else {
		    u[i] = (j = carry + u[i]) / d + ZERO;
		    carry = j % d * NUMBASE;
		}
	    }
	} else {
	    for (i = 0; i <= ulen; i++)
		if (u[i] == point)
		    u[dpu = i] = POINT;
		else
		    u[i] += ZERO;
	}
	u[i] = EOL;
	if (d1 > 0) {
	    u[i + 1] = EOL;
	    u[i + 2] = EOL;
	    i = dpu;
	    for (j = 0; j < d1; j++) {
		u[i] = u[i - 1];
		i--;
	    }
	    u[i] = POINT;
	}
    }
/* remove trailing zeroes */
    i = 0;
    k = (-1);
    while (u[i] != EOL) {
	if (u[i] == POINT)
	    k = i;
	i++;
    }
    i--;
    if (k >= 0) {
	while (u[i] == ZERO && i > k)
	    u[i--] = EOL;
    }
/* remove trailing point */
    if (u[i] == POINT)
	u[i] = EOL;
/* remove leading zeroes */
    while (u[0] == ZERO) {
	i = 0;
	while ((u[i] = u[i + 1]) != EOL)
	    i++;
    }
    if (u[0] == EOL) {
	u[0] = ZERO;
	u[1] = EOL;
	mi = 0;
    }
    if ((mi || plus) && (u[0] != ZERO)) {
	if (mi != plus) {
	    i = stlen (u) + 1;
	    do {
		u[i] = u[i - 1];
		i--;
	    } while (i > 0);
	    u[0] = MINUS;
	}
	if (plus)
	    add (u, vv);
    }
    stcpy (uu, u);
    return;
}					/* end div() */
/******************************************************************************/
void
power (a, b)				/* raise a to the b-th power */
	char   *a,
	       *b;
{
    char    c[STRLEN + 2/*was 257*/];
    char    d[4*(STRLEN + 1)/*was 1024*/];/* 257 should be sufficient, but somewhere there */

/* is a memory leak resulting in wrong results   */
/* with fractional powers, e.g. 2**(3/7)         */
/* even a value of 513 is too small              */
    char    e[STRLEN + 2/*was 257*/];
    register long i;
    register long j;

    if (ierr > OK)
	return;				/* avoid nonsense in recursion */
/* if zero or one there's not much to do */
    if (a[1] == EOL) {
	if (a[0] == ZERO) {
	    if (b[1] == EOL && b[0] == ZERO)
		ierr = DIVER;
	    return;
	}				/* undef */
	if (a[0] == ONE)
	    return;
    }
    if (b[0] == MINUS) {
	power (a, &b[1]);
	if (ierr == MXSTR) {
	    a[0] = ZERO;
	    a[1] = EOL;
	    ierr = OK;
	    return;
	}
	stcpy (c, a);
	a[0] = ONE;
	a[1] = EOL;
	div (a, c, '/');
	return;
    }
    if (b[1] == EOL) {
	switch (b[0]) {
	case ZERO:
	    a[0] = ONE;
	    a[1] = EOL;
	case ONE:
	    return;
	case TWO:
	    stcpy (c, a);
	    mul (a, c);
	    return;
	}
    }
/* look for decimal point */
    e[0] = EOL;
    i = 0;
    while (b[i] != EOL) {
	if (b[i] == POINT) {
	    if (a[0] == MINUS) {
		ierr = DIVER;
		return;
	    }				/* undefined case */
	    if (b[i + 1] == FIVE && b[i + 2] == EOL) {	/* half-integer: extra solution */
		if (i) {
		    stcpy (c, b);
		    add (b, c);
		    power (a, b);
		    if (ierr > OK) {
			a[0] = ONE;
			a[1] = EOL;
			return;
		    }
		}
		g_sqrt (a);
		return;
	    }
	    stcpy (e, &b[i]);
	    b[i] = EOL;
	    break;
	}
	i++;
    }
    stcpy (d, a);
    i = intexpr (b);
    if (ierr == MXNUM)
	return;
/* do it with a small number of multiplications                       */
/* the number of multiplications is not optimum, but reasonably small */
/* see donald e. knuth "The Art of Computer Programming" Vol.II p.441 */
    if (i == 0) {
	a[0] = ONE;
	a[1] = EOL;
    } else {
	j = 1;
	while (j < i) {
	    j = j * 2;
	    if (j < 0) {
		ierr = MXNUM;
		return;
	    }
	}
	if (i != j)
	    j = j / 2;
	j = j / 2;
	while (j) {
	    stcpy (c, a);
	    mul (a, c);
	    if (i & j) {
		stcpy (c, d);
		mul (a, c);
	    }
	    j = j / 2;
	    if (ierr == MXNUM)
		return;
	}
	if (e[0] == EOL)
	    return;
    }
/* non integer exponent */
/* state of computation at this point: */
/* d == saved value of a */
/* a == d^^int(b); */
/* e == frac(b); */
    {
	char    Z[STRLEN + 2/*was 257*/];

/* is fraction the inverse of an integer? */
	Z[0] = ONE;
	Z[1] = EOL;
	stcpy (c, e);
	div (Z, c, '/');
	i = 0;
	for (;;)
	{
	    if ((j = Z[i++]) == EOL) {
		j = intexpr (Z);
		break;
	    }
	    if (j != POINT)
		continue;
	    j = intexpr (Z);
	    if (Z[i] == NINE)
		j++;
	    break;
	}
	Z[0] = ONE;
	Z[1] = EOL;
	lintstr (c, j);
	div (Z, c, '/');
/* if integer */

	if (stcmp (Z, e) == 0) {
	    stcpy (Z, d);
	    root (Z, j);
	    if (ierr <= OK) {
		mul (a, Z);
		return;
	    }				/* on error try other method */
	    ierr = OK;
	}
	Z[0] = ONE;
	Z[1] = EOL;
	zprecise += 2;
	for (;;)
	{
	    c[0] = TWO;
	    c[1] = EOL;
	    mul (e, c);
	    g_sqrt (d);
	    if (e[0] == ONE) {
		e[0] = ZERO;
		numlit (e);
		stcpy (c, d);
		mul (Z, c);
		round (Z, zprecise);
	    }
	    if (e[0] == ZERO)
		break;
	    i = 0;
	    j = (d[0] == ONE ? ZERO : NINE);
	    for (;;) {
		++i;
		if (d[i] != j && d[i] != '.')
		    break;
	    }
	    if (d[i] == EOL || (i > zprecise))
		break;
	}
	zprecise -= 2;
	mul (a, Z);
	round (a, zprecise + 1);
    }
    return;
}					/* end power() */
/******************************************************************************/
void
g_sqrt (a)				/* square root */
	char   *a;

{
    register i,
            ch;

    if (a[0] == ZERO)
	return;
    if (a[0] == MINUS) {
	ierr = DIVER;
	return;
    }
    if (ierr > OK)
	return;				/* avoid nonsense in recursion */
    {
	char    tmp1[STRLEN +2 /*was 257*/],
	        tmp2[STRLEN +2 /*was 257*/],
	        XX[STRLEN +2 /*was 257*/],
	        XXX[STRLEN +2 /*was 257*/];

	stcpy (XX, a);
/* look for good initial value */
	if (a[0] > ONE || (a[0] == ONE && a[1] != POINT)) {
	    i = 0;
	    while ((ch = a[i++]) != EOL) {
		if (ch == POINT)
		    break;
	    }
	    if ((i = (i + 1) / 2))
		a[i] = EOL;
	} else if (a[0] != ONE) {
	    a[0] = ONE;
	    a[1] = EOL;
	}
/* "Newton's" algorithm with quadratic convergence */
	zprecise++;
	do {
	    stcpy (XXX, a);
	    stcpy (tmp1, XX);
	    stcpy (tmp2, a);
	    div (tmp1, tmp2, '/');
	    if (ierr > OK)
		break;
	    add (a, tmp1);
	    tmp2[0] = TWO;
	    tmp2[1] = EOL;
	    div (a, tmp2, '/');
	} while (comp (a, XXX));
	zprecise--;
	return;
    }
}					/* end g_sqrt() */
/******************************************************************************/
void
root (a, n)				/* n.th root */
	char   *a;
	long    n;

{
    register i,
            ch;

    if (a[0] == ZERO)
	return;
    if (a[0] == MINUS || n == 0) {
	ierr = DIVER;
	return;
    }
    if (ierr > OK)
	return;				/* avoid nonsense in recursion */
    {
	char    tmp1[STRLEN +2/*was 257*/],
	        tmp2[STRLEN +2/*was 257*/],
	        XX[STRLEN +2/*was 257*/],
	        XXX[STRLEN +2/*was 257*/];
	short   again;

	stcpy (XX, a);
/* look for good initial value */
	if (a[0] > ONE || (a[0] == ONE && a[1] != POINT)) {
	    i = 0;
	    while ((ch = a[i++]) != EOL && ch != POINT) ;
	    if ((i = (i + n - 2) / n) > 0) {
		a[0] = THREE;
		a[i] = EOL;
	    }
	} else if (a[0] != ONE) {
	    a[0] = ONE;
	    a[1] = EOL;
	}
/* "Newton's" algorithm with quadratic convergence */

	if (zprecise <= 3)
	    again = 0;			/* speedup div with small zprec. */
	else {
	    again = zprecise;
	    zprecise = 2;
	}
      second:;
	zprecise++;
	for (;;) {
	    stcpy (XXX, a);
	    lintstr (tmp1, n - 1);
	    stcpy (tmp2, a);
	    power (tmp2, tmp1);
	    stcpy (tmp1, XX);
	    div (tmp1, tmp2, '/');
	    if (ierr > OK)
		break;
	    lintstr (tmp2, n - 1);
	    mul (a, tmp2);
	    add (a, tmp1);
	    lintstr (tmp2, n);
	    div (a, tmp2, '/');
	    stcpy (tmp2, a);
	    div (XXX, tmp2, '/');
	    tmp2[0] = ONE;
	    if (zprecise <= 0)
		tmp2[1] = EOL;
	    else {
		tmp2[1] = POINT;
		for (i = 2; i < zprecise; i++)
		    tmp2[i] = ZERO;
		tmp2[i++] = FIVE;
		tmp2[i] = EOL;
	    }
	    if (!comp (XXX, tmp2))
		continue;
	    if (zprecise <= 0)
		break;
	    tmp2[0] = POINT;
	    for (i = 1; i < zprecise; i++)
		tmp2[i] = NINE;
	    tmp2[i - 1] = FIVE;
	    tmp2[i] = EOL;
	    if (comp (tmp2, XXX))
		break;
	}
	zprecise--;
	if (again) {
	    zprecise = again;
	    again = 0;
	    goto second;
	}
	return;
    }
}					/* end root() */
/******************************************************************************/
int
numlit (str)
/** str is interpreted as a MUMPS number
					 * and is converted to canonical form
					 * return value: 1=EUR, 4=DM, 0=other */
	char   *str;
{
    long   j,
            mi = 0,
            pointx = (-1),
            expflg = 0;
    long    val,
            exp = 0L;
    int     result;
    register i = 0;
    register ch;

    result = 0;
    if (str[0] < ONE) {
	if (str[0] == EOL) {
	    str[0] = ZERO;
	    str[1] = EOL;
	    return (0);
	}
/* compact signs */
	while (str[i] == PLUS || str[i] == MINUS)
	    if (str[i++] == MINUS)
		mi++;
	if ((j = (mi &= 01)))
	    str[0] = MINUS;
/* compact leading zeroes */
	while (str[i] == ZERO)
	    i++;
	if (i > j)
	    stcpy (&str[j], &str[i]);
	if (str[mi] == EOL) {
	    str[0] = ZERO;
	    str[1] = EOL;
	    return (0);
	}
	i = mi;
    }
    while ((ch = str[i]) <= NINE && ch >= ZERO)
	i++;
    if ((result = unit (&str[i]))) {
	if (i == mi) {
	    str[0] = '0';
	    i = 1;
	}
	ch = str[i] = EOL;
    }
    if (ch == EOL)
	return (result);
    if (ch == POINT) {
	pointx = i++;
	while ((ch = str[i]) <= NINE && ch >= ZERO)
	    i++;
	if ((result = unit (&str[i])))
	    ch = str[i] = EOL;
	if (ch == EOL) {
	    i = pointx;
	    goto point0;
	}
    }
/* check for zero mantissa */
    j = 0;
    while (j < i) {
	if (str[j] > ZERO) {
	    j = (-1);
	    break;
	}
	j++;
    }
    if (j >= 0) {
	str[0] = ZERO;
	str[1] = EOL;
	return (result);
    }
/* search for exponent */
    for (; ch != EOL; ch = str[++i]) {
	if (ch <= NINE && ch >= ZERO) {
	    if (expflg) {
		ch -= ZERO;
		val = exp * NUMBASE + ch;
/* check for numeric overflow */
		if (((val - ch) / NUMBASE) != exp) {
		    ierr = MXNUM;
		    return (0);
		}
		exp = val;
	    }
	    continue;
	}
	if (expflg) {
	    if (ch == MINUS) {
		expflg = (-expflg);
		continue;
	    }
	    if (ch == PLUS)
		continue;
	}
	if ((result = unit (&str[i])))
	    ch = str[i] = EOL;
	str[i] = EOL;
	if (ch == 'E' || (lowerflag && ch == 'e')) {
	    expflg++;
	    continue;
	}
	break;
    }
/* append a point at the right end */
    if (expflg) {
	if (pointx < 0) {
	    i = mi;
	    while (str[i] != EOL)
		i++;
	    if (i >= (STRLEN - 1)/*was 254*/) {
		str[STRLEN] = EOL;
		ierr = MXSTR;
		return (0);
	    }
	    str[pointx = i] = POINT;
	    str[++i] = EOL;
	}
/* if exp shift decimal point */
	if (expflg > 0) {
	    while (exp-- > 0) {
		if ((str[pointx] = str[pointx + 1]) == EOL) {
		    if (pointx >= (STRLEN - 1)/*was 254*/)
			break;
		    str[pointx] = ZERO;
		    str[pointx + 1] = str[pointx + 2] = EOL;
		}
		pointx++;
	    }
	    if (pointx >= (STRLEN - 1)/*was 254*/) {
		str[STRLEN] = EOL;
		ierr = MXSTR;
		return (0);
	    }
	    str[pointx] = POINT;
	} else {			/* (expflg<0) */
	    while (exp-- > 0) {
		if (--pointx < 0) {
		    i = pointx = 0;
		    while (str[i++] != EOL) ;
		    if (i >= (STRLEN - 1)/*was 254*/) {
			ierr = MXSTR;
			return (0);
		    }
		    while (i-- > 0)
			str[i + 1] = str[i];
		    str[0] = ZERO;
		}
		str[pointx + 1] = str[pointx];
	    }
	    str[pointx] = POINT;
	}
    }
    if ((i = pointx) >= 0) {
      point0:while (str[++i] != EOL) ;
	i--;
	while (str[i] == ZERO)
	    str[i--] = EOL;		/* remove trailing zeroes */
	if (str[i] == POINT)
	    str[i] = EOL;		/* remove trailing point */
    }
    if (str[mi] == EOL) {
	str[0] = ZERO;
	str[1] = EOL;
    }
    return (result);
}
/******************************************************************************/
int
unit (str)
/**
                                         * str is interpreted as a currency
                                         * symbol
                                         *  return value: 1=EUR, 4=DM, ...
                                         * 0=other */
	char   *str;
{
    char    ch;

    ch = str[0];
    if ((ch < 'A') || (ch > 'P'))
	return 0;
    switch (ch) {
    case 'E':
	if (str[1] == 'U' && str[2] == 'R')
	    return 1;
	if (str[1] == 'S' && str[2] == 'P')
	    return 5;
	return 0;
    case 'D':
	if (str[1] == 'M')
	    return 4;
	if (str[1] == 'E' && str[2] == 'M')
	    return 4;
	return 0;
    case 'A':
	if (str[1] == 'T' && str[2] == 'S')
	    return 2;
	return 0;
    case 'B':
	if (str[1] == 'F' && str[2] == 'R')
	    return 3;
	return 0;
    case 'F':
	if (str[1] == 'F')
	    return 7;
	if (str[1] == 'M' && str[2] == 'K')
	    return 6;
	if (str[1] == 'R' && str[2] == 'F')
	    return 7;
	return 0;
    case 'I':
	if (str[1] == 'E' && str[2] == 'P')
	    return 8;
	if (str[1] == 'T' && str[2] == 'L')
	    return 9;
	return 0;
    case 'N':
	if (str[1] == 'L' && str[2] == 'G')
	    return 10;
	return 0;
    case 'P':
	if (str[1] == 'T' && str[2] == 'E')
	    return 11;
    }
    return 0;
}
/******************************************************************************/
long
intexpr (str)
	char   *str;

/* 'str' is interpreted as integer and converted to int */
{
    {
	register ch;
	register i = 0;
	register long value;
	register long newval;
	short   minus = FALSE;

	if ((ch = str[0]) == MINUS) {
	    ch = str[1];
	    minus = TRUE;
	    i = 1;
	}
	if (ch >= ZERO && ch <= NINE) {
	    value = ch - ZERO;
	    while ((ch = str[++i]) >= ZERO && ch <= NINE) {
		newval = value * NUMBASE;
		if (newval < 0 || ((newval / NUMBASE) != value)) {
		    ierr = MXNUM;
		    return (minus ? -1 : 1);
		};
		newval += ((long) ch - ZERO);
		if (newval < 0) {
		    ierr = MXNUM;
		    return (minus ? -1 : 1);
		};
		value = newval;
	    }
	    if (minus)
		value = (-value);
	    if (ch == EOL)
		return value;
	} else if (ch != ZERO && ch != PLUS && ch != MINUS && ch != POINT)
	    return 0L;
    }
    {
	register ch;
	register i = 0;
	register long value;
	register long newval;
	char    tmp[STRLEN +2/*was 257*/];

	stcpy (tmp, str);
	numlit (tmp);
	i = (tmp[0] == MINUS);
	if (ierr == MXNUM)
	    return (i ? -1 : 1);
	value = 0L;
	while ((ch = tmp[i++]) >= ZERO && ch <= NINE) {
	    newval = value * NUMBASE;
	    if (newval < 0 || ((newval / NUMBASE) != value)) {
		ierr = MXNUM;
		value = 1;
		break;
	    }
	    newval += ((long) ch - ZERO);
	    if (newval < 0) {
		ierr = MXNUM;
		value = 1;
		break;
	    }
	    value = newval;
	}
	if (tmp[0] == MINUS)
	    value = (-value);
	return value;
    }
}					/* end of intexpr */
/******************************************************************************/
short int
tvexpr (str)				/* str is interpreted as truth valued expression */
	char   *str;
{
    if (str[0] > ZERO && str[0] <= NINE) {
	str[0] = ONE;
	str[1] = EOL;
	return TRUE;
    }
    if (str[1] == EOL) {
	str[0] = ZERO;
	return FALSE;
    } {
	register ch;
	register i = 0;
	register pointx = FALSE;
	register sign = FALSE;

	for (;;)
	{
	    if ((ch = str[i]) > ZERO && ch <= NINE) {
		str[0] = ONE;
		str[1] = EOL;
		return TRUE;
	    }
	    i++;
	    if ((ch == PLUS || ch == MINUS) && sign == FALSE)
		continue;
	    sign = TRUE;
	    if (ch == ZERO)
		continue;
	    if (ch == POINT && pointx == FALSE) {
		sign = TRUE;
		pointx = TRUE;
		continue;
	    }
	    str[0] = ZERO;
	    str[1] = EOL;
	    return FALSE;
	}
    }
}
/******************************************************************************/
void
m_op (a, b, op)
	char   *a,
	       *b;
	short   op;
{
    int     atyp,
            btyp;			/* DM/EUR currency types */
    char    tmp[(STRLEN + 1)/*was 256*/];

    switch (op & 0177) {		/* binary operators */
    case '_':
	if (op & 0200)
	    break;			/* NOT_OPERAND */
	if (stcat (a, b) == 0) {
	    ierr = MXSTR;
	}
	return;
    case '=':
	if (stcmp (a, b))
	    *a = ZERO;
	else
	    *a = ONE;
	if (op & 0200)
	    toggle (*a);		/* NOT_OPERAND */
	a[1] = EOL;
	return;
    case '[':
	if (*b == EOL || find (a, b))
	    *a = ONE;
	else
	    *a = ZERO;
	if (op & 0200)
	    toggle (*a);		/* NOT_OPERAND */
	a[1] = EOL;
	return;
    case ']':
	if (*b == EOL) {
	    if (*a == EOL)
		*a = ZERO;
	    else
		*a = ONE;
	}
/* frequent special case */
	else if (stcmp (a, b) <= 0)
	    *a = ZERO;
	else
	    *a = ONE;
	if (op & 0200)
	    toggle (*a);		/* NOT_OPERAND */
	a[1] = EOL;
	return;
    }					/* end switch */
    atyp = numlit (a);
    if (op == '-') {
	stcpy (&tmp[1], b);
	tmp[0] = '-';
	op = '+';
    } else
	stcpy (tmp, b);
    btyp = numlit (tmp);
    switch (op & 0177) {		/* binary operators, NOT OMITTED */
    case '+':
	if (op & 0200) {
	    ierr = ASSIGNER;
	    return;
	}				/* NOT_OPERAND */
#ifdef EUR2DEM
	if (atyp != btyp) {
	    char    tmp2[256];

	    if ((atyp == 0) && (a[0] == '0'))
		atyp = btyp;		/* zero is any currency */
	    if ((btyp == 0) && (tmp[0] == '0'))
		btyp = atyp;		/* zero is any currency */
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
	    break;
	if (atyp != btyp)
	    cond_round (a, zprecise);
	if (atyp)
	    stcat (a, WHR[atyp]);
#endif /* EUR2DEM */
	break;

    case '*':
	if (op & 0200) {
	    ierr = ASSIGNER;
	    return;
	}				/* NOT_OPERAND */
#ifdef EUR2DEM
	if (btyp && atyp == 0) {
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
	if (atyp) {
	    cond_round (a, zprecise);
	    stcat (a, WHR[atyp]);
	}
#endif /* EUR2DEM */
	break;

    case '/':
    case '\\':
    case '#':

	if (op & 0200) {
	    ierr = ASSIGNER;
	    return;
	}				/* NOT_OPERAND */
#ifdef EUR2DEM
	if (atyp != btyp) {
	    char    tmp2[(STRLEN + 1)/*was 256*/];

	    if (atyp && btyp) {
		if (op == '#') {
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
	    } else if (btyp && typemmflag && (*a != '0' || op == '#')) {
		ierr = TYPEMISMATCH;
		return;
	    }
	} else if (op != '#')
	    atyp = 0;
#endif /* EUR2DEM */
	if (tmp[0] == ZERO) {
	    ierr = DIVER;
	    break;
	}
	div (a, tmp, op);
	goto plus02;

    case ' ':
	power (a, tmp);
	break;				/* ' ' stands for power */

    case '>':

#ifdef EUR2DEM
	if (atyp != btyp) {
	    char    tmp2[(STRLEN + 1)/*was 256*/];

	    if ((atyp == 0) && (a[0] == '0'))
		atyp = btyp;		/* zero is any currency */
	    if ((btyp == 0) && (tmp[0] == '0'))
		btyp = atyp;		/* zero is any currency */
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
	    *a = ONE;
	else
	    *a = ZERO;
	if (op & 0200)
	    toggle (*a);		/* NOT_OPERAND */
	a[1] = EOL;
	break;

    case '<':

#ifdef EUR2DEM
	if (atyp != btyp) {
	    char    tmp2[(STRLEN + 1)/*was 256*/];

	    if ((atyp == 0) && (a[0] == '0'))
		atyp = btyp;		/* zero is any currency */
	    if ((btyp == 0) && (tmp[0] == '0'))
		btyp = atyp;		/* zero is any currency */
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
	    *a = ONE;
	else
	    *a = ZERO;
	if (op & 0200)
	    toggle (*a);		/* NOT_OPERAND */
	a[1] = EOL;
	break;

    case '&':

	if (tvexpr (a)) {
	    tvexpr (tmp);
	    *a = *tmp;
	}
	if (op & 0200)
	    toggle (*a);		/* NOT_OPERAND */
	a[1] = EOL;
	break;

    case '!':

	if (tvexpr (a) == FALSE && tvexpr (tmp))
	    *a = ONE;
	if (op & 0200)
	    toggle (*a);		/* NOT_OPERAND */
	a[1] = EOL;
	break;

    default:
	ierr = ASSIGNER;

    }
    return;
}					/* end m_op */
/******************************************************************************/
	/* rounding */
	/* 'a' is assumed to be a 'canonic' numeric string  */
	/* it is rounded to 'digits' fractional digits      */
void
round (a, digits)
	char   *a;
	int     digits;
{
    int     ch,
            i,
            pointpos,
            lena;

    pointpos = -1;
    i = 0;
    i = 0;
    while (a[i] != EOL) {
	if (a[i] == POINT)
	    pointpos = i;
	i++;
    }
    lena = i;
    if (pointpos < 0)
	pointpos = i;
    if ((pointpos + digits + 1) >= i)
	return;				/* nothing to round */
    i = pointpos + digits + 1;
    if (a[i] < FIVE) {
	a[i] = EOL;
	while (a[--i] == ZERO)
	    a[i] = EOL;
	if (a[i] == POINT) {
	    a[i] = EOL;
	    if (i == 0 || (i == 1 && a[0] == MINUS))
		a[0] = ZERO;
	}
	return;
    }
    for (;;)
    {
	if (i >= pointpos)
	    a[i] = EOL;
	else
	    a[i] = ZERO;
	if (--i < (a[0] == MINUS)) {
	    for (i = lena; i >= 0; i--)
		a[i + 1] = a[i];
	    a[a[0] == '-'] = ONE;
	    break;
	}
	if ((ch = a[i]) == POINT)
	    continue;
	if (a[i] < NINE && ch >= ZERO) {
	    a[i] = ++ch;
	    break;
	}
    }
    return;
}					/* end round */

/* End of $Source: /cvsroot-fuse/gump/FreeM/src/operator.c,v $ */
