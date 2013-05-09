%SYSINFO ; A.Trocha; System-Information 01/28/1999 04:16/GMT+1
	; $Source: /cvsroot-fuse/gump/FreeM/mlib/%SYSINFO.m,v $
	; $Revision: 1.3 $ $Date: 2000/02/18 15:13:42 $
	N tab,i,d,t,cnt
	;
	S tab=35
	S maxlines=19
	;
	W !,"**** ",$ZV," - System Information ****",!!
	;
	S cnt=0
	F i=0:1 S t=$T(table+i) Q:t=""  D
	. I $P(t,";",4)'="*" D out($P(t,";",2),@$P(t,";",3))
	. I cnt>(maxlines-1) W !!,"<RETURN>" R x W ! S cnt=0
	W !!,"Done."
	Q

out(str,val)
	N first
	W !,str,$TR($J("",tab-$L(str))," ","."),": " S cnt=cnt+1
	F  Q:$L(val)+tab+2<80  D
	. W $E(val,1,79-tab-2)
	. S val=$E(val,79-tab-1,$L(val))
	. W !,$J("",tab+2) S cnt=cnt+1
	W val
	Q

table	;device status word of terminal;$V(1)
	;current directory;$V(2)
	;global access path;$V(3)
	;DO-GOTO-JOB routine access path;$V(4)
	;ZL-ZS routine access path;$V(5)
	;%global access path;$V(6)
	;%routine DO-GOTO-JOB access path;$V(7)
	;%_routine ZL-ZS access path;$V(8)
	;OPEN/USE/CLOSE path;$V(9)
	;zallock table file;$V(10)
	;lock table file;$V(11)
	;rga (routine global protocol) file;$V(12)
	;hardcopy file;$V(13)
	;journal file;$V(14)
	;journal flag;$S($V(15)=0:"inactive",$V(15)=1:"write",$V(15)=-1:"read")
	;max # of error messages;$V(16)
	;intrinsic z-commands;$V(17);*
	;intrinsic z_functions;$V(18);*
	;intrinsic special variables;$V(19);*
	;break service code;$V(20)
	;size of last global;$V(21);*
	;number of v22_aliases;$V(22);*
	;contents of 'input buffer';$V(23);*
	;number of screen lines;$V(24)
	;DO-FOR-XEC stack ptr;$V(26)
	;DO-FOR-XEC stack ptr (on error);$V(27)
	;number of mumps arguments;$V(30)
	;environment variables;$V(31)
	;maximum size of a loaded routine;$V(32)
	;number of routine buffers;$V(33)
	;routine buffer size auto adjust;$V(34)
	;max. # of conc. open globals;$V(35)
	;max. # of conc. open devices;$V(36)
	;max. size of local sym table;$V(37)
	;symtab size autoadjust;$V(38)
	;max size of usr spec. var.tbl;$V(39)
	;z_svntab autoadjust;$V(40)
	;max size of DO/XEC/FOR/BREAK stack;$V(41)
	;maximum expression stack depth;$V(42)
	;maximum # of patterns;$V(43)
	;# of chars that make a name unique;$V(44)
	;name case sensitivity flag;$V(45)
	;max length of name plus subscr;$V(46)
	;max length of a subscript;$V(47)
	;single user flag;$V(48)
	;lower case everywhere flag;$V(49)
	;direct mode prompt expression;$V(50)
	;default direct mode prompt string;$V(51);*
	;G0 input translation table;$V(52);*
	;G0 output translation table;$V(53);*
	;G1 input translation table;$V(54);*
	;G1 output translation table;$V(55);*
	;partial pattern match flag;$V(60)
	;partial pattern suppl. char;$V(61)
	;random: seed number;$V(62)
	;random: parameter a;$V(63)
	;random: parameter b;$V(64)
	;random: parameter c;$V(65)
	;SIGTERM handling flag;$V(66)
	;SIGHUP handling flag;$V(67)
	;ZSORT/ZSYNTAX flag;$V(70);*
	;ZNEXT/ZNAME flag;$V(71);*
	;ZPREVIOUS/ZPIECE flag;$V(72);*
	;ZDATA/ZDATE flag;$V(73);*
	;old ZJOB vs. new ZJOB flag;$V(79);*
	;7 vs. 8 bit flag;$V(80)
	;PF1 flag;$V(81)
	;order counter;$V(82)
	;text in $ZE flag;$V(83);*
	;path of current routine;$V(84);*
	;path of last global;$V(85);*
	;path of current device;$V(86);*
	;date type definitions;$V(87)
	;date type definitions;$V(88)
	;UNDEF lvn default expression;$V(89);
	;UNDEF gvn default expression;$V(90);
	;missig QUIT expr dflt expr;$V(91)
	;EUR2DEM: type mismatch err;$V(92)
	;zkey production dflt rule def;$V(93)
	;global prefix;$V(96);*
	;global postfix;$V(97);*
	;routine extention;$V(98)
	;timer offset;$V(99)
	;exit status of last kill;$V(100)
	;local $o/$q data value;$V(110);*
	;global $o/$q data value;$V(111);*
	;remember ZL dir on ZS;$V(133)
