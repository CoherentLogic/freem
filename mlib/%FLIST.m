%FLIST	; A.Trocha; File List 01/29/1999 01:47/GMT+1
	; $Source: /cvsroot-fuse/gump/FreeM/mlib/%FLIST.m,v $
	; $Revision: 1.3 $ $Date: 2000/02/18 15:13:41 $
	N exec,fn
	W !!,?20,"FreeM - Host File Lister Utility"
	W !,?26,$ZD,"  ",$ZT
query	W !!,"Current Directory: ",$V(2)
	W !!,"File Name: "
	R fn I fn="" G ende
	W !!
	S exec="!less "_fn
	X exec
	G query
ende	Q
