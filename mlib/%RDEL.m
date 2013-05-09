%RDEL	; A.Trocha ; Routine Delete 01/29/1999 03:41/GMT+1
	; $Source: /cvsroot-fuse/gump/FreeM/mlib/%RDEL.m,v $
	; $Revision: 1.3 $ $Date: 2000/02/18 15:13:42 $
	;
	N path,rtn,exec
	;
	W !!,?20,"FreeM - Routine Delete Utility"
	W !,?25,$ZD,"  ",$ZT
query	W !!,"Routine selector: "
	R rtn
	I rtn="?" D help G query
	I rtn="^D" W ! D ^%RD G query
	I rtn="" G end
	S rtn=$TR(rtn,"^")
	D rexist^%ED(rtn)
	I ^%UTILITY($J,"%ED")=0 W !!,?10,"routine does not exist.",*7 G query
	W !!,"Delete Selected Routine? <N> " R x
	I $TR(x,"y","Y")'="Y" W !!,"No routines deleted." G end
	;
	;--- delete routine <rtn>
	S path=$S($E(rtn)="%":$V(8),1:$V(4))
	I $L(path)>0&($E(path,$L(path))'="/") S path=path_"/"
	I path="" S path="./"
	S exec="!rm "_path_rtn_$V(98)
	X exec
	W !!,"Routine deleted.",*7
	G query
end	W !!
	Q
	;
help	;
	W !!,"enter the routine name to delete or"
	W !,"enter '^D' to display all routine names"
	Q
