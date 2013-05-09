%uxxxx ; FreeM %u utility library - version 0.5.0.1
 ; Jon Diamond - 1999-03-31
 q
 ; 
 ; INITIALISATION
init() n a
 i $d(^%uparams) s a="^%uparams"
 e  s a="^uparams"
 s %ulang=$g(@a@("lang")) i %ulang="" s %ulang=0
 s %uprompt=$g(@a@("prompt")) i %uprompt="" s %uprompt="^ulang"
 s %uwork=$g(@a@("work")) i %uwork="" s %uwork="^uwork("""_$j_""")"
 ; Really should be as below, but needs $system to be implemented
 ;s %uwork=$g(@a@("work"),"^uwork("""_$system_""","""_$j_""")")
 q "0,init^%uxxxx"
 ; 
 ; 
 ; READ WITH PROMPT
read(prompt,result,length,PX,PY,X,Y,timeout) 
 n a,error
 i '$d(%ulang) s error=$$init
 s prompt=$g(prompt)
%read1 i prompt'="" s error=$$write($$lprompt(prompt),$g(PX),$g(PY)) i error q error
 i $g(result)'="" s error=$$write(" <"_result_"> ") i error q error
 ; 
 ; move to X,Y ********
 i $g(X)'="" s $x=$g(X)
 i $g(Y)'="" s $y=$g(Y)
 i $g(timeout)="" d
 . r a
 e  d
 . r a:$g(timeout)
 ; 
 ; **** hitting escape or escape sequence processing???
 i a'="" s result=a
 i a="@" s result=""
 i a="^" q "-1,read^%uxxxx" ; *** Standard abort
 i $g(length),$l(result)>length s error=$$error("Too many characters") g %read1
 ; 
 q "0,read^%uxxxx"
 ; 
 ; 
 ; READ WITH NEWLINE FIRST
readn(prompt,result,length,PX,PY,X,Y,timeout) 
 n error
 i '$d(%ulang) s error=$$init i error q error
 s error=$$writep($g(prompt)) i error q error
 q $$read("",.result,$g(length),"","",$g(X),$g(Y),$g(timeout))
 ; 
 ; 
 ; READN FOR YES/NO
readyn(prompt,result) 
 n error,res,def
 s def=$g(result),def=$$lprompt($s(def=1:"Y",def=0:"N",1:""))
 i '$d(%ulang) s error=$$init i error q error
%readyn1 s res=def
 s error=$$readn(prompt,.res) i error<0 q error
 s res=$$upper(res)
 i res=$$lprompt("Y") s result=1
 e  i res=$$lprompt("N") s result=0
 e  s error=$$write(" - "_$$lprompt("Enter Y or N")) g %readyn1
 q "0,readyn^%uxxxx"
 ; 
 ; Application Validation Error display after Read (of any kind)
rerror(err)  i err?1"?".e q $$write(" "_err)
 q $$write(" - "_$$lprompt(err))
 ; 
 ; WRITE TEXT
write(text,X,Y) 
 n error
 i '$d(%ulang) s error=$$init i error q error
 ; 
 ; move to X,Y ********
 i $g(X)'="" s $x=$g(X)
 i $g(Y)'="" s $y=$g(Y)
 w text
 q "0,write^%uxxxx"
 ; 
 ; 
 ; WRITE WITH NEWLINE
writen(text) 
 w !,text
 q "0,writen^%uxxxx"
 ; 
 ; 
 ; WRITE WITH NEWLINE/PROMPT
writep(text) 
 n error
 i '$d(%ulang) s error=$$init() i error q error
 w !,$$lprompt(text)
 q "0,writep^%uxxxx"
 ; 
 ; 
 ; WRITE WITH FORMAT
writef(format,text) 
 n error,a
 i '$d(%ulang) s error=$$init i error q error
 s format=$g(format)
 f  q:format=""  s a=$p(format,","),format=$p(format,",",2,$l(format,",")) i a?."#"."!"!(a?1"?"1.n) x "w "_a
 w $g(text)
 q "0,writef^%uxxxx"
 ; 
 ; 
 ; Create+write Error message
error(error) 
 i +error=-1 q $$writep("User abort")
 s error=$$writep("Error - "_$$lprompt(error))
 q error
 ; 
 ; LANGUAGE CONVERSION
lprompt(prompt) i $g(prompt)="" q ""
 ; Remove trailing spaces before look-up and re-apply after
 ; Also everything after first -
 ; Everything before ~ is left unchanged and ~ replaced by -
 n s,t,u
 s u="" i prompt["~" s u=$p(prompt,"~")_" - ",prompt=$p(prompt,"~",2,$l(prompt,"~"))
 s t=$f(prompt,"-")-2
 i 't s t=$l(prompt)
 f t=t:-1 q:$e(prompt,t)'=" "
 s s=$e(prompt,t,$l(prompt)),prompt=$e(prompt,1,t-1)
 i prompt'="" s t=$g(@%uprompt@(%ulang,prompt)) i t'="" s prompt=t
 q u_prompt_s
 ; 
 ; ********** UPPER/LOWER CONVERSION NEED TO HANDLE DIFFERENT LANGUAGES/CHARACTERS
upper(text) 
 s text=$tr(text,"abcdefghijklmnopqrstuvwxyz","ABCDEFGHIJKLMNOPQRSTUVWXYZ")
 q text
 ; 
lower(text) 
 s text=$tr(text,"ABCDEFGHIJKLMNOPQRSTUVWXYZ","abcdefghijklmnopqrstuvwxyz")
 q text

