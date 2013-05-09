###
# $Source: /cvsroot-fuse/gump/FreeM/qa/Makefile,v $
# $Revision: 1.3 $ $Date: 2000/02/29 00:46:25 $
#

.SUFFIXES:	.qa
%:	../%.qa
	(../exec_qa "${M_IMPLEMENTATION}" "${IMPLEMENTATION_VERSION}" $@)

##
#	Add tests here... likely this is the only place this Makefile
#	will need to be modified (except in the case of dependencies).
TESTS =\
	001 002 003 004 005 006 007 008 009 010\
	011 012 013 014 015 016 017 018 019 020\
        021 022 023 024 025 026 027 028 029 030\
        031 032 033 034 035 036 037 038 039 040\
        041 042 043 044 045 046 047 048 049 050\
        051 052

all:	../namespace
	if [ ! -r test001.qa/want.out ] ; then make decode-files ; fi
	for i in ${TESTS} ;\
		do (cd test$${i}.qa; make -f ../Makefile test$${i}); done

${TESTS}:
	(cd test${@}.qa; pwd ; make -f ../Makefile test${@})

../namespace:
	./mk_namespace

##
#	Add dependencies for specific tests
#
test002:	test001
test003:	test002
test050:	test003 

##
#	Miscellaneous helper functions


decode-files:
	make all-files CMD=decode

encode-files:
	if ls */*.out */*.err > /dev/null 2>&1		;\
		then make all-files CMD=encode		;\
		else echo "NOTE: already encoded"	;\
	fi

all-files:
	for i in ${TESTS} ;\
		do (cd test$${i}.qa; make -f ../Makefile ${CMD}); done

decode:
	for i in *.ouu *.euu ;\
		do uudecode < $${i} ; done

encode:
	for i in *.out *.err ;\
		do ../encode `pwd` $${i} ; done

clean:	encode-files
	rm -f */test.* */diff.* */*.err */*.out

# End of $Source: /cvsroot-fuse/gump/FreeM/qa/Makefile,v $
