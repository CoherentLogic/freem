%SYSDEV	; A.Trocha ;  [1999/01/29- 7:41:14]
	; $Source: /cvsroot-fuse/gump/FreeM/mlib/%SYSDEV.m,v $
	; $Revision: 1.2 $ $Date: 2000/02/18 15:13:42 $
	;
	;--- set special screen variables
	S %GO="N tmp S tmp=$V(1) U 0:(0::::65) W $C(27,40,48) V 1:tmp"
	S %GF="N tmp S tmp=$V(1) U 0:(0::::65) W $C(27,40,66) V 1:tmp"
	S %C="$C(27,91)_%Y_"";""_%X_""H"""
	S %CLR="$C(27,91)_""2J"""
	Q
