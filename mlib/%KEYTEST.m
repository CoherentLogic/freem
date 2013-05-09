%KEYTEST ; A.Trocha ; test-proggy for keyb-handler [1999/01/29- 6:18:59]
	; $Source: /cvsroot-fuse/gump/FreeM/mlib/%KEYTEST.m,v $
	; $Revision: 1.2 $ $Date: 2000/02/18 15:13:41 $
	N timeout,mode
	S mode=1
	S timeout=""
	W !!,"testprogram for keyboard handler (ESC => quit)"
	W !!,"timeout = none"
	W !,"mode = interpret"
	W !!
	F  D
	. S result=$$^%KEY(timeout,mode)
	. W !,result
	Q
