%PMTERM	; Portable M Terminal Routines
       	; Copyright (C) 2014 Coherent Logic Development LLC
       	QUIT
       	;
	;
CLRSCR	W $C(27)_"[2J"
	S ($X,$Y)=1
	QUIT
	;
	;
HCENTER()
	QUIT ($ZCOLUMNS/2)
	;
	;
VCENTER()
	QUIT ($ZROWS/2)
	;
	;
LOCATE(ROW,COL)
	S $X=COL,$Y=ROW
	QUIT
	;
	;