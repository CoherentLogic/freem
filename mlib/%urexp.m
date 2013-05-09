%urexp ; Routine export utility - version 0.5.0.1
 ; Jon Diamond - 1999-03-31
 ; 
 n error,device,comment,version
 s error=$$init^%uxxxx(),comment="",version="",device=""
 ; 
 s error=$$writen^%uxxxx($t(+1)_"  "_$$^%uxdat)
 k @%uwork
%rsel s error=$$rsel^%urxxx(%uwork) q:error<0
 i $o(@%uwork@(""))="" s error=$$writep^%uxxxx("No Routines selected") q
 ; 
%device i device'="" s error=$$close^%ufxxx(device) i error<0 g %exit1
 s error=$$select^%ufxxx(.device,"out") 
 i error<0 g %rsel
 ; 
%comment s error=$$readn^%uxxxx("Header comment ",.comment)
 i error<0 g %device
 ; 
%version s error=$$readn^%uxxxx("Add version text ",.version)
 i error<0 g %comment
 ; 
 s error=$$go(device,%uwork,comment,version) i error<0 s a=$$error^%uxxxx(error) g %exit
 s error=$$writep^%uxxxx("Routines written - "_+error)
 s error=$$writep^%uxxxx("Finished")
 ; 
%exit s error=$$close^%ufxxx(device) i error<0 ; ??? if error
%exit1 k @%uwork
 q
 ; 
 ; Executable entry point
 ; Needs modifying to allow a selection mask (major changes!)
go(device,select,comment,version)
 n a,b,c,error,cnt
 i version'="",version'?." "1P.e s version=" ; "_version
 s error=$$header^%ufxxx(device,$t(+1),$g(comment)) i error<0 q error
 s a=99999,cnt=0
 f  s a=$o(@select@(a)) q:a=""  d  q:error<0
 . s error=$$write^%ufxxx(device,a) q:error<0
 . s b=$t(@("+1^"_a)) q:b=""  s error=$$write^%ufxxx(device,b_version) q:error<0
 . f c=2:1 s b=$t(@("+"_c_"^"_a)) q:b=""  s error=$$write^%ufxxx(device,b) q:error<0
 . q:error<0
 . s error=$$write^%ufxxx(device,"")
 . s cnt=cnt+1
 i error'<0 s error=$$trailer^%ufxxx(device,$t(+1))
 i error<0 q error
 q cnt_",go^%urexp"
 ; 
 ; 
 ; Alternative entry point with Open/Close included
export(device,select,comment,version) 
 n error
 s error=$$open^%ufxxx(device,"out") q:error<0
 s error=$$go(device,$g(select),$g(comment),$g(version)) q:error<0
 s error=$$close^%ufxxx(device)
 q

