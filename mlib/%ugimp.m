%ugimp ; Global import utility - version 0.5.0.1
 ; Jon Diamond - 1999-03-31
 ;
 w !,$t(+1),"  ",$$^%uxdat
 ;
 n error,select,device,header,comment,yes,cnt,sel
 s error=$$init^%uxxxx,device=""
 ;
 k @%uwork
 ;
%device i device'="" s error=$$close^%ufxxx(device) i err<0 g %exit1
 s error=$$select^%ufxxx(.device,"in") 
 i error<0 g %exit1
 ;
 s error=$$%header i error<0 s a=$$writep^%uxxxx("Header error on input file") g %exit
 s error=$$writep^%uxxxx("Header - "_header) i error<0 g %exit
 s error=$$writep^%uxxxx("Description - "_comment) i error<0 g %exit
 ;
%cont s yes=1
 s error=$$readyn^%uxxxx("Continue? ",.yes)
 i error<0 g %device
 i 'yes g %exit
 ;
%all s all=1
 s error=$$readyn^%uxxxx("Restore all globals? ",.all) 
 i error<0 g %cont
 ;
%go s error=$$go(device,all) i error<0 s a=$$error^%uxxxx(error) g %exit
 s cnt=+error
 s error=$$writep^%uxxxx("Finished")
 s error=$$writep^%uxxxx("Globals processed - "_cnt)
 ;
%exit i device'="" s error=$$close^%ufxxx(device) i error<0 ; ??? if error
%exit1 k @%uwork
 q
 ;
%header()
 s (header,comment)=""
 s error=$$read^%ufxxx(device,"",.header) i error<0 q error
 s error=$$read^%ufxxx(device,"",.comment) i error<0 q error
 q "0,%header^%ugimp"
 ;
 ; Executable entry point
 ; Needs modifying to allow a selection mask (major changes!)
go(device,all) 
 n a,b,name,d,error,cnt,this,replace
 s name="",cnt=0,all=$g(all,1),this=1,replace=""
%go1 s a="",b=""
 s error=$$read^%ufxxx(device,"",.a) i error<0 q error
 s error=$$read^%ufxxx(device,"",.b) i error<0 q error
 ; w !,a,!,b ; *** debugging
 i a'?1"^".e g %go2
 i $p(a,"(",1)'=name d  q:error<0  i this s cnt=cnt+1
 . s (name,replace)=$p(a,"(",1)
 . i all s error=$$writep^%uxxxx("Processing - "_name) q
 . s error=$$readn^%uxxxx("Restore as ",.replace) i error<0 s this=0 q
 . i replace="*" s all=1,replace=name
 . i replace="-" s this=0 q
 . i $e(replace)'="^" s replace="^"_replace
 . s this=1
 i this s @(replace_$e(a,$l(name)+1,999)_"=b") g %go1
 g %go1
 ;
%go2 i a?1"****".e1"****",a=b q cnt_",go^%ugimp"
 q "-104,go^%ugimp,"_a
 ;
 ;
 ; Alternative entry point with Open/Close included
import(device,all) 
 n error
 s error=$$open^%ufxxx(device,"in") q:error<0
 s error=$$go(device,all) q:error<0
 s error=$$close^%ufxxx(device)
 q

