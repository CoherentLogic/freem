%urxxx ; FreeM Routines Utilities Common Subroutines  - version 0.5.0.1
 ; Jon Diamond - 1999-03-19
 q
 ; 
 ; Select a set of Routines in @%uwork
 ; Needs much more work on the selection masks
rsel(work)
 n a,b,c,entries,error,result
 s entries=0
 i $g(work)="" d
 . s work=$g(%uwork) i work="" s work="^uwork("_$j_")"
 . k @work
%rselr s result="",error=$$readn^%uxxxx("Routine ",.result) 
 i error<0 k @work q error
 i result="" q entries_",rsel^%urxxx"
 i result="?" d rseldisp(work) g %rselr
 i result="?^" s error=$$list^%urdir(1) g %rselr
 ; 
 s entries=entries+1,@work@(entries)=result
 f  s a=$p(result,";") q:a=""  s result=$p(result,";",2,$l(result,";")) d
 . i a'?.1"-"1A.30AN.1"*",a'?.1"-"1"%".31AN.1"*" s error=$$writep^%uxxxx("Invalid routine specification - "_a) q
 . i a?1"-"1.e d
 . . i $e(a,$l(a))="*" s b=$e(a,2,$l(a)-1) d %all q
 . . s c=$e(a,2,$l(a)) d %all1
 . e  d
 . . i $e(a,$l(a))="*" s b=$e(a,1,$l(a)-1) d %all q
 . . i $t(@("+1^"_a))="" s error=$$writep^%uxxxx("Routine does not exist - "_a) q
 . . s c=a d %all1
 g %rselr
 ; 
%all s c=b d %all1
 f  s c=$o(^$routine(c)) q:$e(c,1,$l(b))'=b  d %all1
 q
%all1 i $e(a)="-" k @work@(c) q
 i $t(@("+1^"_c))'="" s @work@(c)=c
 q
 ; 
rseldisp(work)
 s a=99999
 s error=$$writep^%uxxxx("Selected routines"),error=$$writen^%uxxxx("") q:error<0
 f  s a=$o(@work@(a)) q:a=""  s error=$$write^%uxxxx(a_"  ")
 q "0,rseldisp^%urxxx"
 ; 
 ; Delete single routine
 ; 
delete(rtn) n path,exec
 S path=$S($E(rtn)="%":$V(8),1:$V(4))
 I $L(path)>0,($E(path,$L(path))'="/") S path=path_"/"
 I path="" S path="./"
 S exec="!rm "_path_rtn_$V(98)
 X exec
 i $t(@("+1^"_rtn))'="" q "-102,delete^%urxxx"
 q "0,delete^%urxxx"
 ; 
 ; Save single routine
 ; 
save(rtn,code)
 ; Unsure code here works for FreeM - OK for Cache
 n a
 s a=""
 x "ZR  f  s a=$o(code(a)) ZI:a'="""" code(a) i a="""" ZS @rtn ZL %urxxx q"
 q "0,save^%urxxx"

