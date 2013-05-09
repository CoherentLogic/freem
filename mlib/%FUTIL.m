%FUTIL	; A.Trocha ; File i/o Library[1999/02/01-20:20:05]
	; $Source: /cvsroot-fuse/gump/FreeM/mlib/%FUTIL.m,v $
	; $Revision: 1.2 $ $Date: 2000/02/18 15:13:41 $
	Q
	;
	;
direx(path) ;--- test if directory exists
	;        0 = does not exist     1 = success
	;
	S $ZT="direx99^"_$ZN
	N tmp
	S tmp=$V(2)
	V 2:path
	V 2:tmp
	Q 1
	Q
direx99	;--- error
	Q 0
	;
	;
mkdir(path) ;--- create directory
	;        0 = error      1 = ok
	;
	N exec
	S exec="!mkdir "_path_">/dev/null"
	X exec
	Q $$direx(path)
	;
	;
rmdir(path) ;--- delete directory   "rm -r"
	; **** take care ****
	N exec
	S exec="!rm -r "_path_">/dev/null"
	X exec
	Q '$$direx(path)
