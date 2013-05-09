%ur ; FreeM Routine Utility menu - version 0.5.0.1
 ; Jon Diamond - 1999-03-31
 ; 
 ; This is one approach, probably not the best one, but it works
 ; for now and is easily maintainable - copes with the named routines not existing!
 ; 
 n a,z,error
 s a=$$init^%uxxxx
 ; 
%disp s error=$$writen^%uxxxx($t(+1)_"  "_$$^%uxdat)
 s error=$$writep^%uxxxx("Option")
 ; 
 f z=1:1 s a=$t(%menu+z) q:a=" q"  i $t(@$p(a,";",3))'="" s error=$$writep^%uxxxx(z_"~"_$p(a,";",2))
%read s a="",error=$$readn^%uxxxx("Choose option ",.a) i error<0 q
 i a="" q
 i a?1.2n s a=$p($t(%menu+a),";",3) i a?1"^".e,$t(@a)'="" d @a g %disp
 s error=$$rerror^%uxxxx("??")
 g %read
 ; 
%menu ;Routine Compare;^%urcom
 ;Routine Copy;^%urcop
 ;Routine Directory;^%urdir
 ;Routine Delete;^%urdel
 ;Routine Edit;^%uredi
 ;Routine Export;^%urexp
 ;Routine Import;^%urimp
 ;Routine Lister;^%urlis
 ;Routine Search/Replace;^%ursea
 q

