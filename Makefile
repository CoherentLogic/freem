###
# $Source: /cvsroot-fuse/gump/FreeM/Makefile,v $
# $Revision: 1.4 $ $Date: 2000/02/18 15:13:41 $
#


all:
	(cd src; make install)

clean:
	(cd bin; make clean)
	(cd src; make clean)
	rm -rf namespace
	rm -f mlib/^%E mlib/^%SYS
	rm -f mak.log

# End of $Source: /cvsroot-fuse/gump/FreeM/Makefile,v $
