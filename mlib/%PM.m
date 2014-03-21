%PM	; Portable M Startup & Init Routine
	; Copyright (C) 2014 Coherent Logic Development LLC
	N DEFNS,RETVAL
	V 50:"!,$G(^%SYS(""NAMESPACE"",$V(200))),""> "",$C(27,91,63,50,53,104)"
	S DEFNS=$$DEFAULT^%PMNS()
	S RETVAL=$$SWITCH^%PMNS(DEFNS)
	W #,$ZVERSION,!
	W " Copyright (C) 2014 Coherent Logic Development LLC",!
	W " Copyright (C) 1998 MUG Deutschland",!,!
	W " Default Namespace:  ",$G(^%SYS("NAMESPACE",$$DEFAULT^%PMNS())),!,!
	QUIT
	;
	;