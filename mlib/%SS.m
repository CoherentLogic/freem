%SS	; A.Trocha; System Job Status [1999/01/29- 3:42:04]
	; $Source: /cvsroot-fuse/gump/FreeM/mlib/%SS.m,v $
	; $Revision: 1.3 $ $Date: 2000/02/18 15:13:42 $
	;
	N exec,s,pid,tty,stat,idle,prg,ss,ss1,i,tab,tab2
	;
	S tab=8
	S tab2=5
	W !!,?20,"FreeM - System Job Status"
	W !,?23,$ZD,"  ",$ZT
	D ss
	;
	;--- show status
	W !!,?23,"Partitions in Use: ",$G(ss)
	W !
	W !,?tab,"PID/JOB    TTY      STATUS       IDLE       NAME" 
	W !,?tab,"-------   -----   ----------   --------   --------"
	F i=1:1:$G(ss) D
	. S s=ss(i)
	. S pid=$P(s,"'"),stat=$P(s,"'",3)
	. W !,?tab,$$lb(pid_$S(pid=$J:"*",1:""),10),$$lb("TTY"_$P(s,"'",2),8)
	. W $$lb($S(stat="S":"Sleeping",stat="R":"Running",1:"Unknown"),13)
	. W $$lb($P(s,"'",4),11),$P(s,"'",5)
	W !,?tab,"-------   -----   ----------   --------   --------"
	W !!
	W !,?tab2,"Max Parition Size/ Used Partition Size:   ",$V(37),"/",$S
	W !,?tab,"Number of Routine Buffers:   ",$V(33)
	W !,?tab,"  Max Size for Routine:   ",$V(32)
	W !!
	Q
	;
lb(str,len) Q str_$J("",len-$L(str))
	;
ss	N exec,s,prg,pid,tty,stat,idle
	;--- get system status
	K %
	S exec="!<ps -xa|grep "_$V(30,1)
	X exec
	F i=1:1:% D
	. S s=$G(%(i))
	. S prg=$TR($E(s,23,$L(s))," ")
	. I prg'=$V(30,1) Q
	. S pid=$TR($E(s,1,7)," ")
	. S tty=$TR($E(s,8,10)," ")
	. S stat=$TR($E(s,11,12)," ")
	. S idle=$TR($E(s,13,22)," ")
	. S ss=$G(ss)+1
	. S ss(ss)=pid_"'"_tty_"'"_stat_"'"_idle_"'"_prg
	. S ss1(pid)=ss
	K %
	Q
