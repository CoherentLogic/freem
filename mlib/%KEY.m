%KEY(to,mode) ; A.Trocha ; keyb-handler [1999/01/29- 6:01:56]
	; $Source: /cvsroot-fuse/gump/FreeM/mlib/%KEY.m,v $
	; $Revision: 1.2 $ $Date: 2000/02/18 15:13:41 $
	; <to> timeout (sec) default: no timeout
	; <mode> 0 or "" or n.d : return values
	;        1 : interpret
	;
	N buf,x,exec
	;
	S to=$G(to)
	I to="" S to=-1,exec="R *x"
	E  S exec="R *x:"_to
	;
	S buf=""
	;
get	X exec I '$T Q ""
	;
	D add(x)
	;
loop	R *x:0 I '$T G interp
	D add(x)
	G loop
	;
interp	;--- interpret buffer and return string
	I $G(mode)=1 Q $$^%KEYINT(buf)
	Q buf
	;
add(x)	;--- add key-value to buffer
	I buf'="" S buf=buf_"-"
	S buf=buf_x
	Q
