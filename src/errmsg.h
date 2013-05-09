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
    "too much parameters\201",			/* TOOPARA	*/
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
    "out of memory\201"				/* MEMOV */
};
#endif /* EM_ENGLISH */

