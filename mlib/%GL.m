%GL	; A.Trocha; Global List Utility 01/28/1999 02:55/GMT+1
	; $Source: /cvsroot-fuse/gump/FreeM/mlib/%GL.m,v $
	; $Revision: 1.3 $ $Date: 2000/02/18 15:13:41 $
	;
	N gl,orig,index,i,str
	N %TIM,%TIM1,%TIM2
	D INT^%T
	W !,?20,"FreeM - Global Lister Utility"
	W !,?24,"xx-JAN-99  "_%TIM1
query	W !!,"Global selector: ^"
	R gl
	S gl="^"_$TR(gl,"^"_$C(34))
	I $E(gl,$L(gl))="(" S gl=$E(gl,1,$L(gl)-1)
	I $P(gl,"(")="^" Q
	I '$D(@$P(gl,"(")) W !!," ..Global does not exist",*7 G query
	S ref=$TR($P(gl,"(",2),")")
	I ref'="" D
	. F i=1:1 S str=$P(ref,",",i) Q:str=""  I str'=+str S $P(ref,",",i)=$C(34)_str_$C(34)
	. S gl=$P(gl,"(")_"("_ref_")"
	S orig=gl I $F(orig,")") S orig=$E(orig,1,$L(orig)-1)
	W !
	I $D(@gl)-11#2=0 W !,gl,"=",$C(34),@gl,$C(34)
	F  S gl=$Q(@gl) Q:gl=""!($E(gl,1,$L(orig))'=orig)  D
	. W !,gl,"=",$C(34),@gl,$C(34)
	G query
