%urimp ; Routine Import Utility - version 0.5.0.1
 ; Jon Diamond - 1999-03-31
 ;
 w !,$t(+1),"  ",$$^%uxdat()
 ;
 n error,select,device,header,comment,yes,cnt,sel
 s error=$$init^%uxxxx(),device=""
 ;
 k @%uwork
 ;
%device i device'="" s error=$$close^%ufxxx(device) i err<0 g %exit1
 s error=$$select^%ufxxx(.device,"in") 
 i error<0 g %exit1
 ;
 s error=$$%header i error<0 s a=$$writep^%uxxxx("Header error on input file") g %exit
 s error=$$writep^%uxxxx("Header - "_header) i error<0 g %exit
 s error=$$writep^%uxxxx("Description - "_comment) i error<0 g %exit
 ;
%cont s yes=1
 s error=$$readyn^%uxxxx("Continue? ",.yes)
 i error<0 g %device
 i 'yes g %exit
 ;
%all s all=1
 s error=$$readyn^%uxxxx("Restore all routines? ",.all) 
 i error<0 g %cont
 ;
%go s error=$$go(device,all) i error<0 s a=$$error^%uxxxx(error) g %exit
 s cnt=+error
 s error=$$writep^%uxxxx("Finished")
 s error=$$writep^%uxxxx("Routines restored - "_cnt)
 ;
%exit i device'="" s error=$$close^%ufxxx(device) i error<0 ; ??? if error
%exit1 k @%uwork
 q
 ;
%header()
 s (header,comment)=""
 s error=$$read^%ufxxx(device,"",.header) i error<0 q error
 s error=$$read^%ufxxx(device,"",.comment) i error<0 q error
 q "0,%header^%urimp"
 ;
 ; Executable entry point
 ; Needs modifying to allow a selection mask (major changes!)
go(device,all)
 n name,d,error,cnt,this,replace,line
 s cnt=0,all=$g(all,1)
%go1 s name=""
 s error=$$read^%ufxxx(device,"",.name) i error<0 q error
 i name="" g %go1
 i name'?1"%".30an,name'?1a.30an g %go2
 s replace=name,this=1
 i all s error=$$writep^%uxxxx("Processing - "_name) g %go1b
%go1a s error=$$readn^%uxxxx("Restore as ",.replace) i error<0 q error
 i this,replace="*" s all=1,replace=name
 i this,replace="-" s this=0
 i this,replace'?1a.30an,replace'?1"%".30an s error=$$rerror^%uxxxx("Invalid routine name") g %go1a
%go1b k line 
 f d=1:1 s line="",error=$$read^%ufxxx(device,"",.line) q:error<0!(line="")  s line(d)=line i d=1,$p(line," ")=name s $p(line(d)," ",1)=replace
 i this s error=$$save^%urxxx(replace,.line) q:error<0 error  s cnt=cnt+1
 g %go1
 ;
%go2 i name?1"****".e1"****" q cnt_",go^%urimp"
 s error=$$error^%uxxxx("Invalid routine name - "_name) 
 g %go1
 ;
 ;
 ; Alternative entry point with Open/Close included
import(device,all)
 n error
 s error=$$open^%ufxxx(device,"in") q:error<0
 s error=$$go(device,all) q:error<0
 s error=$$close^%ufxxx(device)
 q

