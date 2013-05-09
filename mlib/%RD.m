%RD	; A.Trocha; Routine Directory 01/29/1999 01:30/GMT+1
	; $Source: /cvsroot-fuse/gump/FreeM/mlib/%RD.m,v $
	; $Revision: 1.3 $ $Date: 2000/02/18 15:13:41 $
	; this version is very, very beta
	; todo: LOCKING!!
	;
	N rnorm,rsys,xdir,anz,ext
	;
	;--- path to non-% routines
	S rnorm=$V(4)
	;
	;--- path to %-routines
	S rsys=$V(8)
	;
	;--- get routine extention
	S ext=$V(98)
	;
	W !,?20,"ROUTINE DIRECTORY  "_$ZD
	W !,?23," OF FreeM      "_$ZT
	W !!
	;
	S anz=0
	;
	;--- get and output %-routines
	K %
	S xdir="!<"_$$dircmd()_" "_$$convpath(rsys)_"*"_ext_" 2>/dev/null"
	S $ZT="error^"_$ZN
	X xdir D show($$convpath(rsys),0)
	;
	;--- get and output non-% routines
	K %
	S xdir="!<"_$$dircmd()_" "_$$convpath(rnorm)_"*"_ext_" 2>/dev/null"
	X xdir D show($$convpath(rnorm),1)
	;
end	W !,$J(anz,8)," - Routines",!!
	K %
	Q
	;
show(path,m) ;--- show routines
	;    m=0 show %routines  ;   m=1 show non% routines
	N i,out
	F i=1:1:% D
	. I $G(m)=0&('$F($G(%(i)),"%")) Q
	. I $G(m)=1&($F($G(%(i)),"%")) Q
	. S out=$G(%(i))
	. I path'="" S out=$E(out,$L(path)+1,$L(out))
	. S out=$E(out,1,$L(out)-$L(ext))
	. W $$lb("^"_out) S anz=anz+1
	Q
	;
convpath(dir)
	;--- convert path
	N sl
	S sl=$$slash()
	I dir="" Q ""
	I dir="."_sl Q ""
	I $E(dir,$L(dir))'=sl Q dir_sl
	Q dir
	;
lb(str)	;---
	Q $E(str,1,9)_$J("",10-$L(str))
	;
slash() ;--- get the OS specific directory delimiter (slash)
	Q "/"
	;
dircmd() ;
	;--- get the OS specific directory command
	;--- hmm!? how do I know which OS?
	Q "ls"
	;
error	;--- error - trap
	W !,$ZE,!!
	K % Q
