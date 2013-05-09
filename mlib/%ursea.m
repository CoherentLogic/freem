%ursea ; FreeM Routine Search - version 0.5.0.1
 ; Jon Diamond - 1993-03-31
 ; 
 n error,device,search,replace,cnt
 s error=$$init^%uxxxx,device=""
 ; 
 s error=$$writen^%uxxxx($t(+1)_"  "_$$^%uxdat) 
 k @%uwork
%rsel s error=$$rsel^%urxxx(%uwork) q:error<0
 i $o(@%uwork@(""))="" s error=$$writep^%uxxxx("No Routines selected") q
 ; 
%device i device'="" s error=$$close^%ufxxx(device) i error<0 g %exit1
 s error=$$select^%ufxxx(.device)
 i error<0 g %rsel
 ; 
 k search,replace
 s cnt=0
%search s error=$$readn^%uxxxx("Search for ",.search)
 i error<0 g %device
 s error=$$readn^%uxxxx("Replace with ",.replace) 
 i error<0 g %search
 ; 
 s cnt=cnt+1,search(cnt)=search i replace'="" s replace(cnt)=replace
 g %search
 ; 
%go s error=$$go(device,%uwork,.search,.replace) i error<0 s a=$$error^%uxxxx(error) g %exit
 s error=$$writep^%uxxxx("Routines modified - "_+error)
 s error=$$writep^%uxxxx("Finished")
 ; 
%exit s error=$$close^%ufxxx(device) i error<0 ; ??? if error
%exit1 k @%uwork
 q
 ; 
 ; Executable entry point
 ; Needs modifying to allow a selection mask (major changes!)
go(device,select,search,replace)
 n a,c,cnt,error,f,mod,new,old,r,s
 s error=$$write^%ufxxx(device,"Routine Search/Replace "_$$^%uxdat()) q:error<0
 f r=1:1 q:'$d(search(r))  s error=$$write^%ufxxx(device,search(r)) q:error<0  s error=$$write^%ufxxx(device,"  >>> "_$g(replace(r))) q:error<0
 s a=99999,cnt=0
 f  s a=$o(@select@(a)) q:a=""  d  q:error<0
 . s error=$$write^%ufxxx(device,a) q:error<0
 . s mod=0 k new
 . f c=1:1 s (new,old)=$t(@("+"_c_"^"_a)) q:new=""  d  q:error<0
 . . f r=1:1 q:'$d(search(r))  s s=search(r) i new[s d
 . . . s f=$f(new,s),new=$e(new,1,f-$l(s)-1)_$g(replace(r))_$e(new,f,$l(new))
 . . s new(c)=new
 . . i new=old q
 . . s error=$$write^%ufxxx(device,c_"^"_a_$j("",30-$l(c)-$l(a))_" "_old) q:error<0
 . . i $o(replace("")) s error=$$write^%ufxxx(device,$j("",31)_new) q:error<0
 . . s error=$$write^%ufxxx(device,"")
 . . s mod=1,new(c)=new
 . q:error<0
 . i mod d
 . . s error=$$save^%urxxx(a,.new)
 . . i error<0 s error=$$write^%uxxxx(device,"*** Error in saving routine ***") q
 . . s error=$$write^%ufxxx(device,""),cnt=cnt+1 q:error<0
 i error<0 q error
 q cnt_",go^%ursea"
 ; 
 ; 
 ; Alternative entry point with Open/Close included
search(device,select,search,replace)
 n error
 s error=$$open^%ufxxx(device) q:error<0
 s error=$$go(device,select,.search,.replace) q:error<0
 s error=$$close^%ufxxx(device) q:error<0
 q "0,search^%ursea"

