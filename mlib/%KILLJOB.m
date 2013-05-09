%KILLJOB ; A.Trocha; killjob utility [1999/01/29- 4:28:46]
	; $Source: /cvsroot-fuse/gump/FreeM/mlib/%KILLJOB.m,v $
	; $Revision: 1.3 $ $Date: 2000/02/18 15:13:41 $
	N ss,ss1,pid
	W !!,?20,"FreeM - Job Termination Utility"
	W !,?23,$ZD,"  ",$ZT
query	W !!,"Job number to be halted? "
	R pid
	I pid="?" D help G query
	I pid="^L" D ^%SS G query
	I pid="" G end
	D ss^%SS
	I $G(ss1(pid))="" W !,"Invalid job number!",*7 G query
	V 100:pid
	W !!,"killed...",*7
	G query
end	W !! Q
	;
help	W !!,"enter job number to be halted or"
	W !,"enter '^L' to list all jobs..."
	Q
