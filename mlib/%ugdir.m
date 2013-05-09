%ugdir ; Global Directory  - version 0.5.0.1
 ; A.Trocha - 01/29/1999 00:44/GMT+1
 ; $Source: /cvsroot-fuse/gump/FreeM/mlib/%ugdir.m,v $
 ; $Revision: 1.1 $ $Date: 2000/02/18 15:13:42 $
 ;
 ; this version is very, very beta
 ;
 ; todo: LOCKING!!
 ;
 N no,error
 ;
 s error=$$init^%uxxxx 
 s error=$$writen^%uxxxx($t(+1)_"  "_$$^%uxdat)
 s error=$$writen^%uxxxx("")
 ;
 s no=$$out(0)+$$out(1)
 ; 
 s error=$$writep^%uxxxx("Number of globals - "_$J(no,8))
 Q
 ;
 ;--- get and output %-globals
out(m) n %
 ;--- path to %-globals
 S $ZT="%error^%ugdir"
 ;
 X "!<"_$$dircmd_" "_$$convpath($v($s(m=0:6,1:3)))_"^* 2>/dev/null"
 q $$show(m)
 ;
 ;
 ;
show(m) ;--- show globals
 ;    m=0 show %global  ;   m=1 show non% globals
 ;    do not output ^$<xxxxxx>
 N i,glb
 s glb=0
 F i=1:1:% D
 . I $G(m)=0,'$F($G(%(i)),"%") Q
 . I $G(m)=1,$F($G(%(i)),"%") Q
 . I $F($G(%(i)),"$") Q
 . W $$lb("^"_$P($G(%(i)),"^",2)) S glb=glb+1
 Q glb
 ;
convpath(dir) ;--- convert path
 N sl
 S sl=$$slash
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
dircmd() ;--
 ;--- get the OS specific directory command
 ;--- hmm!? how do I know which OS?
 Q "ls"
 ;
%error ;--- error - trap
 W !,$ZE,!!
 Q
 ;
list() q $$out(1)_",list^%ugdir"

