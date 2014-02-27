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

            INSTALL_DIR = ../bin

               INCLUDES = \
			  errmsg.h\
			  mpsdef.h\
			  mpsdef0.h\
			  version.h

                 CFLAGS = -g -O -DHACK_NOXLATE # -Wall # too much yet

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
	cc ${CFLAGS} compact.o ${LIB}	-o gcompact
gfix:		fix.o ${LIB}
	cc ${CFLAGS} fix.o ${LIB}		-o gfix
gl:		gl.o ${LIB}
	cc ${CFLAGS} gl.o ${LIB}		-o gl
glocks:		locks.o ${LIB}
	cc ${CFLAGS} locks.o ${LIB}		-o glocks
grestore:	restore.o ${LIB}
	cc ${CFLAGS} restore.o ${LIB}	-o grestore
gverify:	verify.o ${LIB}
	cc ${CFLAGS} verify.o ${LIB}		-o gverify
mumps:		mumps.o ${LIB}
	cc ${CFLAGS} mumps.o ${LIB}		-o mumps

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
