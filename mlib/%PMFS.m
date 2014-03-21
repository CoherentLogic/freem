%PMFS	; Portable M Filesystem Routines
	; Copyright (C) 2014 Coherent Logic Development LLC
	QUIT
	;
	;
DIREXIST(PATH)
	S $ZTRAP="DIREXERR^%PMFS"
	N ORIGPATH S ORIGPATH=$V(2)
	V 2:PATH
	V 2:ORIGPATH
	QUIT 1
	QUIT
	;
DIREXERR
	QUIT 0
	;
	;
MKDIR(PATH)
	N CMD S CMD="!mkdir "_PATH_" > /dev/null"
	X CMD
	QUIT $$DIREXIST(PATH)
	;
	;
RMDIR(PATH)
	N CMD S CMD="!rm -r "_PATH_" > /dev/null"
	X CMD
	QUIT '$$DIREXIST(PATH)
	;
	;