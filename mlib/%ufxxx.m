%ufxxx ; File i/o Library - version 0.5.0.1
 ; A.Trocha [1999/02/01-20:20:05]
 ; Utility common file handling subroutines ; Jon Diamond ; 1999-02-18
 ; $Source: /cvsroot-fuse/gump/FreeM/mlib/%ufxxx.m,v $
 ; $Revision: 1.1 $ $Date: 2000/02/18 15:13:42 $
 Q
 ; 
 ; 
direx(path) ;--- test if directory exists
 ;        0 = does not exist     1 = success
 ; 
 S $ZT="%direx99^"_$ZN
 N tmp
 S tmp=$V(2)
 V 2:path
 V 2:tmp
 Q 1
 Q
%direx99 ;--- error
 Q 0
 ; 
 ; 
mkdir(path) ;--- create directory
 ;        0 = error      1 = ok
 ; 
 N exec
 S exec="!mkdir "_path_">/dev/null"
 X exec
 Q $$direx(path)
 ; 
 ; 
rmdir(path) ;--- delete directory   "rm -r"
 ; **** take care ****
 N exec
 S exec="!rm -r "_path_">/dev/null"
 X exec
 Q '$$direx(path)
 ; 
 ; SELECT DEVICE
 ; 
select(device,type,params) 
 n error,d,ntype
 s d=$g(device),type=$g(type),params=$g(params) i type="" s type="in"
%select1 
 s device=d,error=$$readn^%uxxxx($s(type="in":"Input",type="in/out":"Input/Output",1:"Output")_" device: ",.device)
 i error<0 q error
 s ntype=type
 ; 
 ; **** add stuff for parameters etc.??????
 ; **** is it possible to validate for a device name???
 ; **** may want to output to pseudo-device, e.g. global/file
%select2 
 s error=$$open(device,ntype,params,0)
 i error'<0 q "0,select^%ufxxx"
 i +error=-1 q error
 i +error'=-103 s a=$$write(""," - "_error) g %select1
 ; 
 ; Now try again if we didn't try and open the file as a new one
 i ntype'="in",ntype'="new" s ntype="new" g %select2
 s a=$$rerror^%uxxxx("Device not available")
 g %select1
 ; 
 ; 
 ; OPEN DEVICE
 ; 
open(device,type,params,timeout) 
 ; ***** error trapping probably needed here to allow for
 ; ***** device selection
 ; ***** and parameters to be incorrect in some way
 s device=$g(device),params=$g(params)
 i device=""!(device=0)!(device=$i) q "0,open^%ufxxx"
 s timeout=$g(timeout)
 ; ***** handle types = in, in/out, out
 s type=$g(type,"in")
 i type'="out" s params="R"_params
 i type'="in" s params="W"_params
 i type="new" s params="N"_params
 i timeout="" o device:params
 e  o device:params:timeout e  q "-103,open^%ufxxx"
 q "0,open^%ufxxx"
 ; 
 ; 
 ; CLOSE DEVICE
close(device) 
 s device=$g(device)
 i device=""!(device=0)!(device=$i) q "0,close"
 ; ***** do we need to add parameters on close here???
 c device
 q "0,close^%ufxxx"
 ; 
 ; READ WITH PROMPT
 ; 
read(device,prompt,result,length,PX,PY,X,Y,timeout) 
 n a,error
 i '$d(%ulang) s error=$$init^%uxxxx()
 s dev=$i,device=$g(device)
 s prompt=$g(prompt)
 i prompt'="" s error=$$write(device,$$lprompt^%uxxx(prompt),$g(PX),$g(PY)) i error q error
 i dev'=device,device'=0,device'="" u device
 i $g(result)'="" s error=$$write(device," <"_result_"> ") i error q error
 ; 
 ; move to X,Y ********
 i $g(X)'="" s $x=$g(X)
 i $g(Y)'="" s $y=$g(Y)
 i $g(timeout)="" d
 . r a
 e  d
 . r a:$g(timeout)
 ; 
 ; **** hitting escape or escape sequence processing???
 u dev
 i a'="" s result=a
 i a="@" s result=""
 i $g(length),$l(result)>length q "-101,read"
 ; 
 q "0,read^%ufxxx"
 ; 
 ; 
 ; WRITE TEXT
write(device,text,X,Y) 
 n error
 i '$d(%ulang) s error=$$init() i error q error
 s dev=$i,device=$g(device)
 i dev'=device,device'=0,device'="" u device
 ; 
 ; move to X,Y ********
 i $g(X)'="" s $x=$g(X)
 i $g(Y)'="" s $y=$g(Y)
 w text,!
 u dev
 q "0,write^%ufxxx"
 ; 
 ; Write header
header(device,type,comment) 
 n error
 s error=$$write^%ufxxx(device,"**** "_type_" ~ "_$$^%uxdat($h,1)_" ****") i error<0 q error
 s error=$$write^%ufxxx(device,"**** "_$g(comment)_" ****") i error q error
 q "0,header^%ufxxx"
 ; 
 ; Write trailer
trailer(device,type) 
 n a,error
 s a="**** "_$g(type)_" ~ "_$$^%uxdat($h,1)_" ****"
 s error=$$write^%ufxxx(device,"") i error<0 q error
 s error=$$write^%ufxxx(device,a) i error q error
 q "0,header^%ufxxx"

