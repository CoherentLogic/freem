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
 * module for error messages
 * 
 */

#ifdef EM_GERMAN
int     SIflag[MAXDEV + 1] =
{0, 0, 0, 0, 0};			/* SI/SO flag                    */
#endif /* EM_GERMAN */

#ifdef EM_ENGLISH
int     SIflag[MAXDEV + 1] =
{1, 1, 1, 1, 1};			/* SI/SO flag                    */
#endif EM_ENGLISH

char    errmes[MAXERR][ERRLEN] =	/* error messages                  */
#ifdef EM_GERMAN
{
    "\201",					/* OK		*/
    "Interrupt\201",				/* INRPT	*/
    "BREAK point\201",				/* BKERR	*/
    "Nicht Standard\201",			/* NOSTAND	*/
    "Variable nicht definiert\201",		/* UNDEF	*/
    "Marke nicht gefunden\201",			/* LBLUNDEF	*/
    "fehlender Operand\201",			/* MISSOPD	*/
    "fehlender Operator\201",			/* MISSOP	*/
    "unbekannter Operator\201",			/* ILLOP	*/
    "Anfuehrungszeichen zuwenig\201",		/* QUOTER	*/
    "Komma erwartet\201",			/* COMMAER	*/
    "Zuweisung '=' erwartet\201",		/* ASSIGNER	*/
    "ungueltiges Argument\201",			/* ARGER	*/
    "Leerzeichen erwartet\201",			/* SPACER	*/
    "Klammerfehler\201",			/* BRAER	*/
    "Falsche Ebene\201",			/* LVLERR	*/
    "Division durch Null\201",			/* DIVER	*/
    "unbekannte Funktion\201",			/* ILLFUN	*/
    "falsche Anzahl Argumente\201",		/* FUNARG	*/
    "ZTRAP Fehler\201",				/* ZTERR	*/
    "$NEXT/$ORDER Fehler\201",			/* NEXTER	*/
    "$SELECT Fehler\201",			/* SELER	*/
    "unbekanntes Kommando\201",			/* CMMND	*/
    "Argumentenliste\201",			/* ARGLIST	*/
    "ungueltiger Ausdruck\201",			/* INVEXPR	*/
    "ungueltige Referenz\201",			/* INVREF	*/
    "Zeichenkette zu lang\201",			/* MXSTR	*/
    "zuviele Parameter\201",			/* TOOPARA	*/
    "Einheit nicht offen\201",			/* NOPEN	*/
    "Einheit nicht definiert\201",		/* NODEVICE	*/
    "Zugriff nicht erlaubt\201",		/* PROTECT	*/
    "Global unzulaessig\201",			/* GLOBER	*/
    "Datei nicht gefunden\201",			/* FILERR	*/
    "Programm-Ueberlauf\201",			/* PGMOV	*/
    "Keller-Ueberlauf\201",			/* STKOV	*/
    "Variablenbereich-Ueberlauf\201",		/* STORE	*/
    "Eingabe nicht moeglich\201",		/* NOREAD	*/
    "Ausgabe nicht moeglich\201",		/* NOWRITE	*/
    "Programm nicht gefunden\201",		/* NOPGM	*/
    "ungueltige Kurz-Referenz\201",		/* NAKED	*/
    "ungueltiger Index\201",			/* SBSCR	*/
    "ungueltige MUMPS Zeile beim Einfuegen\201",/* ISYNTX	*/
    "Datenbankfehler\201",			/* DBDGD	*/
    "Job abgebrochen\201",			/* KILLER	*/
    "Hangup-Signal\201",			/* HUPER	*/
    "Numerischer Ueberlauf\201",		/* MXNUM	*/
    "Funktion liefert keinen Wert\201",		/* NOVAL	*/
    "inkompatible Typen\201",			/* TYPEMISMATCH	*/
    "CANNOT ALLOCATE MEMORY\201"		/* MEMOV */
    
};
#endif /* EM_GERMAN */

#ifdef EM_ENGLISH
{
    "\201",					/* OK		*/
    "interrupt\201",				/* INRPT	*/
    "BREAK point\201",				/* BKERR	*/
    "non standard syntax\201",			/* NOSTAND	*/
    "variable not found\201",			/* UNDEF	*/
    "label not found\201",			/* LBLUNDEF	*/
    "missing operand\201",			/* MISSOPD	*/
    "missing operator\201",			/* MISSOP	*/
    "unrecognized operator\201",		/* ILLOP	*/
    "unmatched quotes\201",			/* QUOTER	*/
    "comma expected\201",			/* COMMAER	*/
    "equals '=' expected\201",			/* ASSIGNER	*/
    "argument not permitted\201",		/* ARGER	*/
    "blank ' ' expected\201",			/* SPACER	*/
    "unmatched parentheses\201",		/* BRAER	*/
    "level error\201",				/* LVLERR	*/
    "divide by zero\201",			/* DIVER	*/
    "function not found\201",			/* ILLFUN	*/
    "wrong number of function arguments\201",	/* FUNARG	*/
    "ZTRAP error\201",				/* ZTERR	*/
    "$NEXT/$ORDER error\201",			/* NEXTER	*/
    "$SELECT error\201",			/* SELER	*/
    "illegal command\201",			/* CMMND	*/
    "argument list\201",			/* ARGLIST	*/
    "invalid expression\201",			/* INVEXPR	*/
    "invalid reference\201",			/* INVREF	*/
    "string too long\201",			/* MXSTR	*/
    "too many parameters\201",			/* TOOPARA	*/
    "unit not open\201",			/* NOPEN	*/
    "unit does not exist\201",			/* NODEVICE	*/
    "protected file\201",			/* PROTECT	*/
    "global not permitted\201",			/* GLOBER	*/
    "file not found\201",			/* FILERR	*/
    "program overflow\201",			/* PGMOV	*/
    "stack overflow\201",			/* STKOV	*/
    "symbol table overflow\201",		/* STORE	*/
    "file won't read\201",			/* NOREAD	*/
    "file won't write\201",			/* NOWRITE	*/
    "routine not found\201",			/* NOPGM	*/
    "illegal naked reference\201",		/* NAKED	*/
    "illegal subscript\201",			/* SBSCR	*/
    "insert syntax\201",			/* ISYNTX	*/
    "database degradation\201",			/* DBDGD	*/
    "job kill signal\201",			/* KILLER	*/
    "hangup signal\201",			/* HUPER	*/
    "numeric overflow\201",			/* MXNUM	*/
    "function returns no value\201",		/* NOVAL	*/
    "type mismatch\201",			/* TYPEMISMATCH	*/
      "out of memory\201",				/* MEMOV */
      "M1: naked indicator undefined\201",
      "M2: invalid combination with $FNUMBER code atom\201",
      "M3: $RANDOM seed less than 1\201",
      "M4: no true condition in $SELECT\201",
      "M5: line reference less than zero\201",
      "M6: undefined local variable\201",
      "M7: undefined global variable\201",
      "M8: undefined intrinsic special variable\201",
      "M9: divide by zero\201",
      "M10: invalid pattern match range\201",
      "M11: no parameters passed\201",
      "M12: invalid line reference (negative offset)\201",
      "M13: invalid line reference (line not found)\201",
      "M14: line level not 1\201",
      "M15: undefined index variable\201",
      "M16: argumented QUIT not allowed\201",
      "M17: argumented QUIT required\201",
      "M18: fixed length READ not greater than zero\201",
      "M19: cannot copy a tree or subtree into itself\201",
      "M20: line must have a formal parameter list\201",
      "M21: algorithm specification invalid\201",
      "M22: SET or KILL to ^$GLOBAL when data in global\201",
      "M23: SET or KILL to ^$JOB for non-existent job number\201",
      "M24: change to collation algorithm while subscripted local variables defined\201",
      " \201",
      "M26: non-existent environment\201",
      "M27: attempt to rollback a transaction that is not restartable\201",
      "M28: mathematical function, parameter out of range\201",
      "M29: SET or KILL on structured system variable not allowed by implementation\201",
      "M30: reference to global variable with different collating sequence within a collating algorithm\201",
      "M31: control mnemonic used for device without a mnemonic space selected\201",
      "M32: control mnemonic used in user-defined mnemonic space which has no associated line\201",
      "M33: SET or KILL to ^$ROUTINE when routine exists\201",
      " \201",
      "M35: device does not support mnemonic space\201",
      "M36: incompatible mnemonic spaces\201",
      "M37: READ from device identified by the empty string\201",
      "M38: invalid structured system variable subscript\201",
      "M39: invalid $NAME argument\201",
      "M40: call-by-reference in JOB actual parameter\201",
      "M41: invalid LOCK argument within a transaction\201",
      "M42: invalid QUIT within a transaction\201",
      "M43: invalid range value ($X, $Y)\201",
      "M44: invalid command outside of a transaction\201",
      "M45: invalid GOTO reference\201",
      "M57: more than one defining occurrence of label in routine\201",
      "M58: too few formal parameters\201"
};
#endif /* EM_ENGLISH */

