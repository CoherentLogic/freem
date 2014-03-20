###
# $Source: /cvsroot-fuse/gump/FreeM/src/Makefile,v $
# $Revision: 1.5 $ $Date: 2000/02/22 17:48:23 $
# /*                           *
#                             * *
#                            *   *
#                       ***************
#                        * *       * *
#                         *  MUMPS  *
#                        * *       * *
#                       ***************
#                            *   *
#                             * *
#                              *
#
# Shalom ha-Ashkenaz, 1998/06/18 CE
#
# compile modules for FreeMUMPS
#
# */

	    CC = gcc

            INSTALL_DIR = ../bin

               INCLUDES = \
			  errmsg.h\
			  mpsdef.h\
			  mpsdef0.h\
			  version.h

                 CFLAGS = -g -O0 -DHACK_NOXLATE -DUSE_GETOPT -DDEBUG_GBL #-DDEBUG_SYM # -Wall # too much yet

                    LIB = mumps.a

               MAINRTNS =\
			  compact.o\
			  fix.o\
			  gl.o\
			  locks.o\
			  mumps.o\
			  restore.o\
			  verify.o

                LIBRTNS =\
			  xecline.o\
			  expr.o\
			  symtab.o\
			  global.o\
			  operator.o\
			  service.o\
			  strings.o\
			  views.o

                EXERTNS =\
			  gcompact\
			  gfix\
			  gl\
			  glocks\
			  grestore\
			  gverify\
			  mumps

all:		${EXERTNS}

install:	${EXERTNS}
	mv ${EXERTNS} ${INSTALL_DIR}

gcompact:	compact.o ${LIB}
	${CC} ${CFLAGS} compact.o ${LIB}	-o gcompact
gfix:		fix.o ${LIB}
	${CC} ${CFLAGS} fix.o ${LIB}		-o gfix
gl:		gl.o ${LIB}
	${CC} ${CFLAGS} gl.o ${LIB}		-o gl
glocks:		locks.o ${LIB}
	${CC} ${CFLAGS} locks.o ${LIB}		-o glocks
grestore:	restore.o ${LIB}
	${CC} ${CFLAGS} restore.o ${LIB}	-o grestore
gverify:	verify.o ${LIB}
	${CC} ${CFLAGS} verify.o ${LIB}		-o gverify
mumps:		mumps.o ${LIB}
	${CC} ${CFLAGS} mumps.o ${LIB}		-o mumps

${LIB}:	${LIBRTNS}
	ar rv  ${LIB} ${LIBRTNS}
	ranlib ${LIB}

version.o:	Makefile version.c
version.h:	Makefile version
	./version > version.h

${MAINRTNS}:	Makefile ${INCLUDES}
${LIBRTNS}:	Makefile ${INCLUDES}

tags:	*.[ch]
	ctags *.[ch]

clean:
	rm -f core tags version version.[ho]
	rm -f ${EXERTNS} ${MAINRTNS} ${LIB} ${LIBRTNS}

# End of $Source: /cvsroot-fuse/gump/FreeM/src/Makefile,v $
