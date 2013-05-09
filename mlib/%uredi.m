%uredi ; Initialize Editor Global  - version 0.5.0.1
 ; A.Trocha; 01/29/1999 03:07/GMT+1
 ; $Source: /cvsroot-fuse/gump/FreeM/mlib/%uredi.m,v $
 ; $Revision: 1.1 $ $Date: 2000/02/18 15:13:42 $
 ; after having run this proggy you can invoke the editor by
 ; entering X ^%ue
 ;
 ; by default VI is used as editor. If you would like another editor
 ; please set the ^%SYS("EDITOR") Global accordingly
 ;
 q
 ;
init n a
 s a=$$init^%uxxxx()
 s a=$g(@%uparams@("routinneeditor"),"^%ue")
 S @(a_"=""N a,prg,f,exec,e,ed d init0^%uredi X @ed@(1)""")
 S @a@(1)="W !!,""EDIT PROGRAM NAME: "" R prg Q:prg=""""  X @ed@(2)"
 S @a@(2)="S prg=$TR(prg,""^""),f=$V(4) X @ed@(3)"
 S @a@(3)="S:$A(prg)=37 f=$V(8) X @ed@(4)"
 S @a@(4)="S:$L(f)>0&($A(f,$L(f))'=47) f=f_$C(47) X @ed@(5)"
 S @a@(5)="D rexist^%uredi(prg) S e=@%uwork@(""%ED"") X @ed@(5.5)"
 S @a@(5.5)="X:e=0 ^%ue(6) X:e=1 @ed@(12)"
 S @a@(6)="W !,""CREATE A NEW PROGRAM? <Y>: "" X @ed@(7)"
 S @a@(7)="R a S a=$TR(a,""y"",""Y"") S:a="""" a=""Y"" X @ed@(8)"
 S @a@(8)="Q:a'=""Y""  X @ed@(9)"
 S @a@(9)="ZR  X @ed@(10)"
 S @a@(10)="ZI prg_"" ; [""_$ZD_""-""_$ZT_""]"" ZS @prg X @ed@(11)"
 S @a@(11)="W !,prg_"" created!"",*7 H 1 X @ed@(12)"
 S @a@(12)="S exec=""!"_$$ed^%uredi()_" ""_f_prg_$V(98) X @ed@(13)"
 S @a@(13)="x exec S tmp=$V(1) U 0:(0::::65) X @ed@(14)"
 S @a@(14)="V 23:""zl ""_prg_"" zl ""_prg_"" V 1:tmp""_$C(13)"
 I $G(QUIET)=1 Q
 ;
 W !,"Full Screen Editor installed."
 W !,"To use the editor enter:  X "_a,!!
 Q
 ;
 ;
rexist(rtn) ;--- check if a given routine exists
 N exec
 S $ZT="error^%uredi"
 S exec="ZL "_rtn_" G cont^%uredi" X exec
cont S @%uwork@($ZN)=1 Q
error S @%uwork@($ZN)=0 Q
 ;
init0 s a=$$init^%uxxxx(),ed=$g(@%uparams@("routinneeditor"),"^%ue")
 q
 ;
ed() ;--- which editor to use  (this is used be ^%ue)
 N ed
 S ed=$G(@%uparams@("EDITOR"))
 I $TR(ed," ")="" Q "vi"
 Q ed
 ;
chksum() ;--- chksum... not used yet... wanted to use it to determine if a
 ;---- routine was changed to change to date+time in the header line
 N sum,i,line
 f i=0:1 S line=$T(+i) Q:line=""  S sum=$G(sum)+$ZCRC(line)
 Q sum
 ;
 ;
other ;--- entry-point for %SYSGEN to change default editor
 N x,erg,QUIET
 S %ta=+$G(%ta)
other0 W !,?%ta,"Editor: " R x
 I x="" W !,"Not changed!",! Q
 W !,?%ta,"Sure? <Y> "
 S erg=$$ask^%MUTIL("Y")
 I erg=0 G other0
 S ^%SYS("EDITOR")=x
 W !,?%ta,"Changed.",!
 S QUIET=1 D ^%uredi
 Q
 ;
 ;
current ;--- show the current editor
 N ed
 S %ta=+$G(%ta)
 S ed=$G(^%SYS("EDITOR"))
 W !!,?%ta,"Current Editor: "
 W $S(ed'="":ed,1:"vi")
 Q

