%MEN(y) ; A.Trocha ; Menu (up/down/choice) [1999/01/31-21:25:56]
	; $Source: /cvsroot-fuse/gump/FreeM/mlib/%MEN.m,v $
	; $Revision: 1.2 $ $Date: 2000/02/18 15:13:41 $
	; <y> row where to position menue
	; inp> ^%UTILITY(%J,0..n) = choice
	; inp> ^%UTILITY(%J)   = default choice'headline
	; out> as function result chosen <i>
	;	-1 if esc was pressed
	;
	D ^%SYSDEV
	;
	N i,max,df,len,len,cnt,dflt,key,pos,max,head
	S i=""
	;
	;---  get the largest choice
	F  S i=$O(^%UTILITY($J,i)) Q:i=""  D
	. S len=$L(^%UTILITY($J,i)),max=i
	. I len>+$G(maxlen) S maxlen=len
	S head=$P($G(^%UTILITY($J)),"'",2)
	I $L(head)>maxlen S maxlen=$L(head)
	;
	;--- get params to center
	S x=(80-maxlen+2)\2
	;
	;--- get default choice - if none is given use the first entry
	S dflt=+$G(^%UTILITY($J))
	;
	;--- output initial menu
	S i=""
	F  S i=$O(^%UTILITY($J,i)) Q:i=""  D ref(i,dflt)
	D c(x,y+1,$$z(head,maxlen+2))
	;
	;--- draw window
	X %GO
	D c(x-1,y,"l"_$TR($J("",maxlen+2)," ","q")_"k")
	D c(x-1,y+1,"x"),c(x+maxlen+2,y+1,"x")
	F i=0:1:max+1 D c(x-1,y+i+2,"x"),c(x+maxlen+2,y+i+2,"x")
	D c(x-1,y+2,"t"_$TR($J("",maxlen+2)," ","q")_"u")
	D c(x-1,y+max+4,"m"_$TR($J("",maxlen+2)," ","q")_"j")
	X %GF
	;
	;---
	S pos=dflt
	W $C(27,91,63,50,53,108) ; turn off cursor
loop	S key=$$^%KEY("",1)
	I key="UP" I pos>0 D ref(pos,-1) S pos=pos-1 D ref(pos,pos) G loop
	I key="DOWN" I pos<max D ref(pos,-1) S pos=pos+1 D ref(pos,pos) G loop
	I key="CR",$TR($G(^%UTILITY($J,pos))," ")'="" G end
	I key="ESC" S pos=-1 G end
	G loop
	;
ref(rel,akt)	;
	N df
	S df=$S(akt=rel:">",1:" ")
	D c(x,y+rel+3,df_$$z(^%UTILITY($J,rel),maxlen)_$TR(df,">","<"))
	Q
	;
end	;
	W $C(27,91,63,50,53,104) ; turn  cursor back on
	Q pos
	
c(x,y,t) W *27,*91,y,";",x,"H"_$G(t) Q
z(txt,len) ;
	N l,add
	S l=(len-$L($E(txt,1,len)))\2
	S add=len-(l*2+$L($E(txt,1,len)))
	Q $J("",l+add)_$E(txt,1,len)_$J("",l)
