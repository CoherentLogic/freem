%KEYINT(buf) ; A.Trocha ; [1999/01/29- 6:15:25]
	; $Source: /cvsroot-fuse/gump/FreeM/mlib/%KEYINT.m,v $
	; $Revision: 1.2 $ $Date: 2000/02/18 15:13:41 $
	;
	; quick and dirty
	; going to put values in a global (some time ;-))
	;
	I buf="8" Q "BS"
	I buf="13" Q "CR"
	I buf="27" Q "ESC"
	I buf="27-91-65" Q "UP"
	I buf="27-91-66" Q "DOWN"
	I buf="27-91-67" Q "RIGHT"
	I buf="27-91-68" Q "LEFT"
	I buf="27-91-49-126" Q "INS"
	I buf="27-91-51-126" Q "PGUP"
	I buf="27-91-52-126" Q "DEL"
	I buf="27-91-54-126" Q "PGDN"
	Q buf
