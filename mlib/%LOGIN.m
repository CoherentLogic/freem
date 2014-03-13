%LOGIN	;automatically eXecuted at start of MUMPS
	;Shalom ha-Ashkenaz,1998/06/18 CE
	; $Source: /cvsroot-fuse/gump/FreeM/mlib/%LOGIN.m,v $
	; $Revision: 1.3 $ $Date: 2000/02/18 15:13:41 $
ST	W #,$ZVERSION,!
	W " Copyright (C) 1998 Shalom ha-Ashkenaz",!
	W " Copyright (C) 2014 Coherent Logic Development LLC",!,!
 	W " DEFAULT NAMESPACE:  ",$G(^%SYS("NSPACE",$$dflt^%SYSNSP())),!
	D ^%SYS
	Q
