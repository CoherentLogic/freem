%uflis ; FreeM File List - version 0.5.0.1
 ; A.Trocha ; 01/29/1999 01:47/GMT+1
 ; $Source: /cvsroot-fuse/gump/FreeM/mlib/%uflis.m,v $
 ; $Revision: 1.1 $ $Date: 2000/02/18 15:13:42 $
 N exec,fn
 ;
 w !,$t(+1),!,$$^%uxdat
 ;
query W !!,"Current Directory: ",$V(2)
 W !!,"File Name: "
 R fn I fn="" G end
 W !!
 S exec="!less "_fn
 X exec
 G query
end Q

