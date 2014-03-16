/***
 * $Source: /cvsroot-fuse/gump/FreeM/src/version.c,v $
 * $Revision: 1.4 $ $Date: 2000/02/22 17:48:24 $
 */

#include <stdio.h>
#include <string.h>

main()
{
    char   *rev = "$Revision: 1.5 $", *bp;
    int     ln;

    bp = strchr(rev, ' ');		/* Skip out to first space	*/
    if (bp == NULL)			/* Huh?				*/
	bp = " ?.0 ";
    ln = strchr(++bp, ' ') - bp;	/* Limit length to next space	*/

    printf("/***\n");
    printf("/*\tAutomatically generated header, see %s\n", __FILE__);
    printf(" */\n");
    printf("\n");
    printf("#ifndef FREEM_VERSION_H\n");
    printf("#define FREEM_VERSION_H\n");
    printf("#define FREEM_VERSION_ID  \"0.%.*s\"\n",      ln, bp);
    printf("#define FREEM_VERSION_STR \"0.%.*s\\201\"\n", ln, bp);
    printf("#endif/*FREEM_VERSION_H*/\n");
    printf("\n");
    printf("/* End of generated header */\n");

    exit( 0 );
} /* main */


/* End of $Source: /cvsroot-fuse/gump/FreeM/src/version.c,v $ */
