%MUTIL	; A.Trocha ; Library [1999/02/01-20:04:22]
	; $Source: /cvsroot-fuse/gump/FreeM/mlib/%MUTIL.m,v $
	; $Revision: 1.2 $ $Date: 2000/02/18 15:13:41 $
	Q
	;
ask(d)	;
	N x R x S x=$TR(x,"ny ","NY"),d=$TR(d,"ny","NY ")
	I x=""!($E(x)=$E(d)) Q 1
	Q 0
	 
