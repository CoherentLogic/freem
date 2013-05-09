%urdir ; FreeM Routine Directory - version 0.5.0.1
 ; A.Trocha ; 01/29/1999 01:30/GMT+1
 ; $Source: /cvsroot-fuse/gump/FreeM/mlib/%urdir.m,v $
 ; $Revision: 1.1 $ $Date: 2000/02/18 15:13:42 $
 ; this version is very, very beta
 ; todo: LOCKING!!
 ; 
 N no,error
 ; 
 ; 
 ;--- get routine extention
 ; S ext=$V(98)
 ;
 s error=$$init^%uxxxx 
 s error=$$writen^%uxxxx($t(+1)_"  "_$$^%uxdat)
 s error=$$writen^%uxxxx("")
 ; 
 ;--- get and output %-routines
 S $ZT="%error^%urdir"
 s no=$$list(0)+$$list(1)
 ; 
 s error=$$writep^%uxxxx("Number of routines - "_$J(no,8))
 Q
 ; 
 ;--- get and output non-% routines
list(m) ;
 n rnorm,%
 ;--- path to non-% routines
 S rnorm=$V($s(m=1:4,1:8))
 x "!<"_$$dircmd()_" "_$$convpath(rnorm)_"*"_$V(98)_" 2>/dev/null"
 q $$show($$convpath(rnorm),m)_",list^%ugdir"
 ;
show(path,m) ;--- show routines
 ;    m=0 show %routines  ;   m=1 show non% routines
 N i,out,no
 s no=0
 F i=1:1:% D
 . I $G(m)=0&('$F($G(%(i)),"%")) Q
 . I $G(m)=1&($F($G(%(i)),"%")) Q
 . S out=$G(%(i))
 . I path'="" S out=$E(out,$L(path)+1,$L(out))
 . S out=$E(out,1,$L(out)-$L($V(98)))
 . W $$lb("^"_out) S no=no+1
 Q no
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
lb(str) ;---
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
%error ;--- error - trap
 W !,$ZE,!!
 K % Q

