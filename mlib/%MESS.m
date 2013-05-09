%MESS(y,t,len) ; A.Trocha ; display text box [1999/01/31-22:52:31]
	; $Source: /cvsroot-fuse/gump/FreeM/mlib/%MESS.m,v $
	; $Revision: 1.2 $ $Date: 2000/02/18 15:13:41 $
	; <y> row to start with
	; <t> text to display
	; <len> length of text-info ... default: len=$L(t)+2
	;
	N %X,%Y
	I $G(len)="" S len=$L(t)+2
	S %X=80-len\2
	S %Y=y
	X %GO W @%C,"l",$TR($J("",len)," ","q"),"k"
	S %Y=y+1
	W @%C,"x" X %GF W $$z^%MEN(t,len) X %GO W "x"
	S %Y=y+2
	W @%C,"m",$TR($J("",len)," ","q"),"j" X %GF
	Q
