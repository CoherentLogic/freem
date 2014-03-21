%PMMENU	; Portable M Menu API
	; Copyright (C) 2014 Coherent Logic Development LLC
	QUIT
	;
	;
	; Menu Structure
	;  MA("CAPTION")="Menu Caption"
	;  MA("ITEMS")=item-count
	; 
	;  For top-level items:
	;
	;  MA("ITEMS",item-num)="M CODE"
	;  MA("ITEMS",item-num,"CAPTION")="Item caption"
	;	
	;  For submenus:
	;
	;  MA("ITEMS",item-num)=""
	;  MA("ITEMS",item-num,"CAPTION")="Submenu Caption"
	;  MA("ITEMS",item-num,"ITEMS")=subitem-count
	;  MA("ITEMS",item-num,"ITEMS",subitem-num)="M CODE"
	;  MA("ITEMS",item-num,"ITEMS",subitem-num,"CAPTION")="Subitem caption"

	
DISPLAY(MA)
	