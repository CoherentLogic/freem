%CLPM 
 ; Coherent Logic Portable M - Startup & Init Routine
 ; Copyright (C) 2014 Coherent Logic Development LLC
 ; Author: John Willis
 I $D(^%SYS("NOSYS")) Q
 N DEFNS,RETVAL
 D ^%SYSDEV
 V 50:"!,$G(^%SYS(""NSPACE"",$V(200))),""> "",$C(27,91,63,50,53,104)"
 S DEFNS=$$dflt^%SYSNSP()
 I DEFNS=+DEFNS D
 . S RETVAL=$$switch^%SYSNSP(DEFNS)
 . I 'RETVAL W "Error switching to system default namespace."
 W #,$ZVERSION,!
 W " Copyright (C) 2014 Coherent Logic Development LLC",!
 W " Copyright (C) 1998 MUG Deutschland",!,!
 W " Default Namespace:  ",$G(^%SYS("NSPACE",$$dflt^%SYSNSP())),!,!
 Q
