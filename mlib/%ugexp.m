%ugexp ; Global export utility - version 0.5.0.1
 ; Jon Diamond - 1999-03-31
 ;
 w !,$t(+1),"  ",$$^%uxdat
 ;
 n error,device,comment
 s error=$$init^%uxxxx,comment="",device=""
 ;
 k @%uwork
%gsel s error=$$gsel^%ugxxx(%uwork) q:error<0
 i $o(@%uwork@(""))="" s error=$$writep^%uxxxx("No globals selected") q
 ;
%device i device'="" s error=$$close^%ufxxx(device) i error<0 g %exit1
 s error=$$select^%ufxxx(.device,"out")
 i +error=-1 g %gsel
 i error<0 g %exit1
 ;
%comment s error=$$readn^%uxxxx("Header comment ",.comment)
 i +error=-1 g %device
 ;
 s error=$$go(device,%uwork,comment) i error<0 s a=$$error^%uxxxx(error) g %exit
 s error=$$writep^%uxxxx("Finished")
%exit s error=$$close^%ufxxx(device) i error<0 ; ??? if error
%exit1 k @%uwork
 q
 ;
 ; Executable entry point
 ; Needs modifying to allow a selection mask (major changes!)
go(device,select,comment)
 n a,b,c,error,cnt
 s error=$$header^%ufxxx(device,$t(+1),$g(comment)) i error<0 q error
 s a=99999
 f  s a=$o(@select@(a)) q:a=""  d  q:error<0
 . s (b,c)=^(a),cnt=0 i $e(b)'="^" s (b,c)="^"_b
 . i $e(c,$l(c))=")" s $e(c,$l(c))=""
 . s error=$$writep^%uxxxx("Writing global - "_b)
 . f  s b=$q(@b) q:$e(b,1,$l(c))'=c  d  q:error<0
 . . s error=$$write^%ufxxx(device,b) q:error<0
 . . s error=$$write^%ufxxx(device,@b)
 . . s cnt=cnt+1
 i error'<0 s error=$$trailer^%ufxxx(device,$t(+1))
 q error
 ;
 ;
 ; Alternative entry point with Open/Close included
export(device,select,comment) 
 n error
 s error=$$open^%ufxxx(device,"out") q:error<0
 s error=$$go(device,select,$g(comment)) q:error<0
 s error=$$close^%ufxxx(device)
 q

