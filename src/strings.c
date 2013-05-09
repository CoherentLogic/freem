/***
 * $Source: /cvsroot-fuse/gump/FreeM/src/strings.c,v $
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
 * string handling - c_routines
 * 
 */

#include "mpsdef.h"

long int
stlen (source)				/* length of 'source' string in bytes */
	char   *source;
{
    register length = 0;

    while (*source++ != EOL)
	length++;
    return length;
}
/******************************************************************************/
long int
stcpy (dest, source)			/* copy string from 'source' to 'dest' */
	char   *dest,
	       *source;
{
    register count = 0;

    while ((*dest++ = *source++) != EOL)
	count++;
    return count;
}
/******************************************************************************/
void
stcpy0 (dest, source, length)		/* copy exactly 'length' characters */
	char   *dest,			/* from 'source' string to 'dest' */
	       *source;
	long    length;

{
    while (length-- > 0)
	*dest++ = *source++;
    return;
}
/******************************************************************************/
void
stcpy1 (dest, source, length)		/* copy exactly 'length' characters */
	char   *dest,			/* from 'source' string to 'dest' */
	       *source;
	long    length;

{
    while (length-- > 0)
	*dest-- = *source--;
    return;
}
/******************************************************************************/
short int
stcat (dest, source)			/* concatenate string from 'source'   */

/* at the end of the string in 'dest' */
	char    dest[];			/* do not copy more than STRLEN chars */
	char   *source;

{
    register i = 0;

    while (dest[i++] != EOL) ;
    i--;
    while ((dest[i] = *source++) != EOL) {
	if (i++ >= STRLEN) {
	    dest[--i] = EOL;
	    return FALSE;
	}
    }
    return TRUE;
}
/******************************************************************************/
short int
stcmp (str1, str2)			/* compare str1 and str2 */
	char   *str1,
	       *str2;
{
    while (*str1 == *str2) {
	if (*str1 == EOL)
	    return 0;
	str1++;
	str2++;
    }
    return *str1 - *str2;
}
/******************************************************************************/

/* End of $Source: /cvsroot-fuse/gump/FreeM/src/strings.c,v $ */
