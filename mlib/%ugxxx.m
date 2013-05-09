%ugxxx ; FreeM Global Utilities Common Subroutines - version 0.5.0.1
 ; Jon Diamond - 1999-03-31
 q
 ; 
 ; Select a set of Globals in @%uwork
 ; Needs much more work on the selection masks
gsel(work)
 n a,b,c,error,result,entries
 s entries=0
 i $g(work)="" d
 . s work=$g(%uwork) i work="" s work="^uwork("_$j_")"
 . k @work
%gselr s result="",error=$$readn^%uxxxx("Global ",.result) i error<0 k @work q error
 i result="" q entries_",gsel^%ugxxx"
 i result="?" s error=$$gseldisp(work) g %gselr
 i result="?^" s error=$$list^%ugdir g %gselr
 ; 
 s entries=entries+1,@work@(entries)=result
 f  s a=$p(result,";") q:a=""  s result=$p(result,";",2,$l(result,";")) d
 . i a'?.1"-"1A.30AN.1"*",a'?.1"-"1"%".31AN.1"*" s error=$$writep^%uxxxx("Invalid global specification - "_a) q
 . i a?1"-"1.e d
 . . i $e(a,$l(a))="*" s b=$e(a,2,$l(a)-1) d %all q
 . . s c=$e(a,2,$l(a)) d %all1
 . e  d
 . . i $e(a,$l(a))="*" s b=$e(a,1,$l(a)-1) d %all q
 . . i '$d(@("^"_a)) s error=$$writep^%uxxxx("Global does not exist - "_a) q
 . . s c=a d %all1
 g %gselr
 ; 
%all s c=b d %all1
 f  s c=$o(^$global(c)) q:$e(c,1,$l(b))'=b  d %all1
 q
%all1 i $e(a)="-" k @work@(c) q
 i $d(@("^"_c)) s @work@(c)=c
 q
 ; 
gseldisp(work)
 s a=99999
 s error=$$writep^%uxxxx("Selected globals") q:error<0
 f  s a=$o(@work@(a)) q:a=""  s error=$$writen^%uxxxx("^"_a)
 q "0,gseldisp^%ugxxx"

