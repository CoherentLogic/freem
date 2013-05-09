%SYS	; A.Trocha ; FreeM System-Startup [1999/02/01- 7:41:14]
	; $Source: /cvsroot-fuse/gump/FreeM/mlib/%SYS.m,v $
	; $Revision: 1.3 $ $Date: 2000/02/18 15:13:42 $
	;
	I $D(^%SYS("NOSYS")) Q
	;
	K
	N dflt,ok
	;
	;--- load special screen management variables
	D ^%SYSDEV
	;
	;--- setting commandline prompt
	D prompt
	;
	;--- determine which configuration to use
	;S dflt=$$dflt^%SYSCFG()
	;
	;--- chose chonfiguration if there are more than one
	;S ok=$$switch^%SYSCFG()
	;I 'ok W !,*7,*7,"ERROR unable to switch configurations",!!
	;
	;--- determine which namespace to use
	S dflt=$$dflt^%SYSNSP()
	;
	;--- switch namespace to default namespace
	I dflt=+dflt D
	. S ok=$$switch^%SYSNSP(dflt)
	. I 'ok W !,*7,*7,"ERROR unable to switch to default namespace",!!
	;
	;--- check if Editor Global exists, if not - set it up
	I '$D(^%E)&($T(^%ED)'="") D
	. D ^%ED
	. W !,"For more setup options please use the ^%SYSGEN Utility",!
	Q
	;
prompt	V 50:"!,$G(^%SYS(""NSPACE"",$V(200))),"">"",$C(27,91,63,50,53,104)" Q
