%urdel ; FreeM Routine Delete - version 0.5.0.1
 ; Jon Diamond - 1999-3-31
 ; 
 N rtn,error
 ; 
 s error=$$init^%uxxxx
 s error=$$writen^%uxxxx($t(+1)_"  "_$$^%uxdat)
 ; 
%query s rtn=$$rsel^%urxxx(%uwork) q:error<0
 i $o(@%uwork@(""))="" s error=$$writep^%uxxxx("No routines selected") q
 s error=$$rseldisp^%urxxx(%uwork)
 s error=$$go(%uwork)
 q
  
go(select)
 n a,error,cnt
 s a="",cnt=0
 f  s a=$o(@select@(a)) q:a=""  d  q:error<0
 . s error=$$delete^%urxxx(a)
 . i error<0 s error=$$writep^%uxxxx("Routine not deleted - "_a) q
 . s error=$$writep^%uxxxx("Routine deleted - "_a)
 . s cnt=cnt+1
 i error<0 q error
 s error=$$writep^%uxxxx("Total number of routines deleted - "_cnt)
 q error

