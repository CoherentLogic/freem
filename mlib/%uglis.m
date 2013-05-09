%uglis ; Global List Utility - version 0.5.0.1
 ; A.Trocha - 01/28/1999 02:55/GMT+1
 ; $Source: /cvsroot-fuse/gump/FreeM/mlib/%uglis.m,v $
 ; $Revision: 1.1 $ $Date: 2000/02/18 15:13:42 $
 ;
 N gl,orig,i,str,error
 W !,$t(+1),"  ",$$^%uxdat
 s error=$$init^%uxxxx
 ; 
%query s gl="",error=$$readn^%uxxxx("Global selector - ",.gl)
 i error<0 q
 i gl="" q
 S gl="^"_$TR(gl,"^""")
 I $E(gl,$L(gl))="(" S gl=$E(gl,1,$L(gl)-1)
 I '$D(@$P(gl,"(")) s error=$$rerror^%uxxxx("Global does not exist") G %query
 S ref=$TR($P(gl,"(",2),")")
 I ref'="" D
 . F i=1:1 S str=$P(ref,",",i) Q:str=""  I str'=+str S $P(ref,",",i)=""""_str_""""
 . S gl=$P(gl,"(")_"("_ref_")"
 S orig=gl I $F(orig,")") S orig=$E(orig,1,$L(orig)-1)
 s error=$$writen^%uxxxx("")
 I $D(@gl)-11#2=0 s error=$$writen^%uxxxx(gl_"="""_@gl_"""")
 F  S gl=$Q(@gl) Q:gl=""!($E(gl,1,$L(orig))'=orig)  D
 . s error=$$writen^%uxxxx(gl_"="""_@gl_"""")
 G %query

