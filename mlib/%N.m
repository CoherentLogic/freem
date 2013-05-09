%N	; A.Trocha ; choose Namespace [1999/02/01-22:11:25]
	; $Source: /cvsroot-fuse/gump/FreeM/mlib/%N.m,v $
	; $Revision: 1.2 $ $Date: 2000/02/18 15:13:41 $
	N idx,cnt,tab,dflt
	;
	D ^%SYSDEV
	W @%CLR
	;
	D ^%MESS(1,"Choose a namespace to work with:")
	;
	;--- load namespaces into ^%UTILITY global
	K ^%UTILITY
	S idx="",cnt=0
	S dflt=$G(^%SYS("NSPACE"))
	F   S idx=$O(^%SYS("NSPACE",idx)) Q:idx=""  D
	. S ^%UTILITY($J,cnt)=^%SYS("NSPACE",idx)
	. s tab(cnt)=idx
	. I idx=dflt S $P(^%UTILITY($J),"'")=cnt
	. S cnt=cnt+1
	S $P(^%UTILITY($J),"'",2)="Namespaces"
	;
	;--- invoke Menu
	S erg=$$^%MEN(8)
	W !!
	I erg=-1 W !,"Canceled.",! Q
	;
	;--- switch UCI's
	S erg=tab(erg)
	S erg=$$switch^%SYSNSP(erg)
	I erg W !,"Done.",! D prompt^%SYS
	E  W !,"Failed.",!
	Q
