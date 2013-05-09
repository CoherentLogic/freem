%ulstring       ; STRING library - version 0.5.0.1
 ;
 ; Unless otherwise noted, the code below
 ; was approved in document X11/95-11
 ;
 ; If corrections have been applied,
 ; first the original line appears,
 ; with three semicolons at the beginning of the line.
 ;
 ; Then the source of the correction is acknowledged,
 ; then the corrected line appears, followed by a
 ; line containing three semicolons.
 ;
 ;
 ;
PRODUCE(IN,SPEC,MAX) ;
 NEW VALUE,AGAIN,P1,P2,I,COUNT
 SET VALUE=IN,COUNT=0
 FOR  DO  QUIT:'AGAIN
 . SET AGAIN=0
 . SET I=""
 . FOR  SET I=$ORDER(SPEC(I)) QUIT:I=""  DO  QUIT:COUNT<0
 . . QUIT:$GET(SPEC(I,1))=""
 . . QUIT:'($DATA(SPEC(I,2))#2)
 . . FOR  QUIT:VALUE'[SPEC(I,1)  DO  QUIT:COUNT<0
 . . . SET P1=$PIECE(VALUE,SPEC(I,1),1)
 . . . SET P2=$PIECE(VALUE,SPEC(I,1),2,$LENGTH(VALUE))
 . . . SET VALUE=P1_SPEC(I,2)_P2,AGAIN=1
 . . . SET COUNT=COUNT+1
 . . . IF $DATA(MAX),COUNT>MAX SET COUNT=-1,AGAIN=0
 . . . QUIT
 . . QUIT
 . QUIT
 QUIT VALUE
 ;===
 ;
 ;
REPLACE(IN,SPEC) ;
 NEW L,MASK,K,I,LT,F,VALUE
 SET L=$LENGTH(IN),MASK=$JUSTIFY("",L)
 SET I="" FOR  SET I=$ORDER(SPEC(I)) QUIT:I=""  DO
 . QUIT:'($DATA(SPEC(I,1))#2)
 . QUIT:SPEC(I,1)=""
 . QUIT:'($DATA(SPEC(I,2))#2)
 . SET LT=$LENGTH(SPEC(I,1))
 . SET F=0 FOR  SET F=$FIND(IN,SPEC(I,1),F) QUIT:F<1  DO
 . . QUIT:$E(MASK,F-LT,F-1)["X"
 . . SET VALUE(F-LT)=SPEC(I,2)
 . . SET $EXTRACT(MASK,F-LT,F-1)=$TRANSLATE($JUSTIFY("",LT)," ","X")
 . . QUIT
 . QUIT
 SET VALUE="" FOR K=1:1:L DO
 . IF $EXTRACT(MASK,K)=" " SET VALUE=VALUE_$EXTRACT(IN,K) QUIT
 . SET:$DATA(VALUE(K)) VALUE=VALUE_VALUE(K)
 . QUIT
 QUIT VALUE
 ;===
 ;
 ;
FORMAT(V,L) ;
 ;
 ; The code below was approved in document X11/SC13/1998-10
 ;
 NEW C,CD,CM,CS,DP,E,EX,FL,FM,FO,GL,GV1,GV2,GVL,GVM,GX,I,J,K,ST,TV,TY,V1,V2,VP
 ;
 ; Load up Format Directives from ^$FORMAT or ^SYSTEM("FORMAT")
 DO:'$DATA(^$FORMAT) %INFORM
 SET (FM,K)="",EX=0,EXS="EXS"
 ;
 ; Extract the working values from the command string
 DO %PRELOAD
 ;
 ; Process the directives
 DO %EVALU8
 ;
 ; Error Handling
 DO:EX %ERROR
 XECUTE:$LENGTH(EXS) "K "_EXS
 QUIT K
 ;
 ; CM  - Command Array
 ; CS  - Command String
 ; DP  - Decimal Pointer
 ; EX  - Exit Flag
 ; EXS - KILL Exit String
 ; FL  - Field Length
 ; FM  - Format String
 ; FO  - Format Option Array
 ; K   - Return Output String
 ; L   - List of Directives
 ; ST  - String Extraction String
 ; V   - Input Value
 ;
%PRELOAD ; Load the defaults prior to the application of directives
 SET K=""
 ; Load System Defaults
 FOR  SET K=$ORDER(^$SYSTEM("FORMAT",K)) QUIT:K=""  DO
 . SET FO(K)=^$SYSTEM("FORMAT",K)
 . QUIT
 ; Load Process Defaults
 FOR  SET K=$ORDER(^$FORMAT(K)) QUIT:K=""  SET FO(K)=^$FORMAT(K)
 SET (CS,L)=$GET(L)
 ; Load Argument Overrides from the List of Directives
 ; 1. Tokenize the Laterals
 DO:L[""""
 . SET CS=""
 . FOR J=2:2:$LENGTH(L,"""") DO
 . . SET ST=$GET(ST)+1,ST(ST)=$PIECE(L,"""",J)
 . . SET:ST(ST)="" ST(ST)=""""
 . . SET CS=CS_$PIECE(L,"""",J-1)_"%%"_ST_"%%"
 . . QUIT
 . SET CS=CS_$PIECE(L,"""",J+1)
 . QUIT
 ; 2. Evaluate the Directives
 NEW C,L,X
 FOR J=1:1:$LENGTH(CS,":") DO  QUIT:EX
 . SET CD=$PIECE(CS,":",J)
 . SET X=$PIECE(CD,"="),TV=$PIECE(CD,"=",2,999)
 . IF X="" SET EX=1 QUIT
 . ; Uppercase Symbol Names Only
 . SET TY=$TRANSLATE(X,"abcdefghijklm","ABCDEFGHIJKLM")
 . SET TY=$TRANSLATE(TY,"nopqrstuvwxyz","NOPQRSTUVWXYZ")
 . D
 . . ; To bullet proof, Establish an error trap to %ERREX
 . . IF ($LENGTH(TV,"%%")>1) DO %LOADTV SET FO(TY)=TV QUIT
 . . IF TV="" SET FO(TY)="" QUIT
 . . ; 3. Set the Directive in the FO array
 . . SET FO(TY)=@TV
 . . QUIT
 . QUIT
 ; 4) Construct a KILL Exit String for directives not in default list
 NEW C,E
 SET (C,E)=""
 FOR  SET C=$ORDER(FO(C)) QUIT:C=""  SET @C=FO(C),E=E_C_","
 SET EXS=E_"EXS"
 SET:$DATA(FM) FL=$LENGTH(FM)
 QUIT
 ;
 ; DC  - Decimal Character
 ; DP  - Decimal Position
 ; EX  - Abnormal Condition Exit
 ; FL  - Format Length
 ; FM  - Format Mask
 ; GV1 - Integer Portion
 ; GV2 - Fractional Portion
 ; K   - Output Buffer
 ; NOD - No Decimal
 ; SV  - Sign Value (1 = Positive, 0 = Negative)
 ; V   - Input Value
%EVALU8 ; Evaluate the input for loading into the output string
 NEW NOD
 SET SV=1
 SET NOD='(FM["d")
 SET:V<0 SV=0
 SET V1=$PIECE(V,"."),V2=$PIECE(V,".",2),(GV1,GV2)=""
 DO:FM'=""
 . SET FL=$LENGTH(FM),DP=$FIND(FM,"d")-1
 . SET:(DP<1) DP=$LENGTH(FM)
 . QUIT
 NEW C
 DO %GETV1,%GETV2:'(NOD!EX)
 QUIT:EX
 ;
 SET K=GV1_DC_GV2
 SET:NOD K=GV1
 IF $GET(FC)'="" SET:K[" " K=$TRANSLATE(K," ",FC)
 SET:$LENGTH(K)'=FL EX=1
 QUIT
 ;
%GETV1 ; Get the integer portion of the value and lay it in GV1
 NEW CP,SP
 IF $GET(SL)'="" NEW SC SET SC=SL
 ; 1) Set the Integer Portion of the Mask (GVM) and Length (GVL)
 SET GVM=$PIECE(FM,"d"),GVL=$LENGTH(GVM),GL=0
 ; 2) Get the absolute value of V1
 SET:$EXTRACT(V1)="-" V1=$EXTRACT(V1,2,999)
 ; 3) Establish Blank Mask, GV1
 SET GV1=$J("",GVL),VP=$LENGTH(V1),(CP,SP)=1
 ;
 ; Rounding of Integer (NO DECIMAL PORTION)
 SET:$PIECE(FM,"d",2)="" V1=$EXTRACT((V+.5)\1,1,$LENGTH(V1))
 ; 4) Extract value for each position in the mask and set it
 FOR L=GVL:-1:1 SET C=$EXTRACT(GVM,L) DO QUIT:EX
 . SET GX=0
 . DO %TRANSV1
 . QUIT:GX!EX
 . ;
 . SET:GC'=" " $EXTRACT(GV1,L)=GC
 . QUIT
 SET:VP EX=1
 QUIT
 ;
%GETV2 ; Get the fractional portion of the value and lay it in GV2
 NEW CP,SP
 IF $GET(SR)'="" NEW SC SET SC=SR
 SET GVM=$PIECE(FM,"d",2),GVL=$LENGTH(GVM),GL=0,SP=1
 DO:GVL<$LENGTH(V2)  ; Rounding of Decimal
 . NEW J,N
 . SET N=$EXTRACT($TRANSLATE($J("",GVL)," ",0)_5_$TRANSLATE($J("",$LENGTH(V2))," ",0),1,$LENGTH(V2))
 . SET V2=$EXTRACT(V2+N,1,$LENGTH(V2))
 . QUIT
 SET GV2=$J("",GVL),VP=1,CP=1
 FOR L=1:1:GVL SET C=$EXTRACT(GVM,L) DO  QUIT:EX
 . SET GX=0
 . DO %TRANSV2
 . QUIT:GX!EX
 . ;
 . SET:GC'=" " $EXTRACT(GV2,L)=GC
 . QUIT
 QUIT
 ;
 ; C   - Current Mask Character from the FM
 ; CP  - Character Position
 ; L   - Position within
 ; VP  - Value Position
 ; (integer - Right to Left, fraction - Left to Right)
%TRANSV1 ; Translate the value into the mask
 SET (GC,GL)=" "
 QUIT:"x "[C
 ;
 ; Value Completed, Apply Currency/Float/etc, if requested
 IF 'VP DO
 . IF "c"[C  DO  QUIT
 . . SET:$GET(CP)="" CP=$LENGTH(CS)
 . . SET GC=$EXTRACT(CS,CP),CP=CP-1
 . . SET:CP<1 CP=$LENGTH(CS)
 . . QUIT
 . IF GVM["f"  DO  QUIT
 . . NEW F,I,LI,LX,X,Q
 . . SET X=" ",LI=L,LX=0
 . . DO  ; Identify the Value Represented
 . . . IF GVM["+"!(GVM["-") DO  QUIT
 . . . . SET:GVM["+" X="+"
 . . . . SET:V<0 X="-"
 . . . . QUIT
 . . . IF GVM["(" DO:V<0  QUIT
 . . . . SET X=" ",LX=1
 . . . . QUIT
 . . . QUIT
 . . FOR I=L:1:GVL SET Q=$EXTRACT(GV1,I) QUIT:Q?1N  QUIT:("("_DC)[Q  DO
 . . . SET F=$EXTRACT(GVM,I),LI=I
 . . . SET:"fs("[F $EXTRACT(GV1,I)=X
 . . . QUIT
 . . SET BYE=1
 . . SET:LX $EXTRACT(GV1,LI)="("
 . . QUIT
 . QUIT
 IF C="+" SET GC="+" SET:'SV GC="-" SET GL=GC QUIT
 IF C="-" SET:'SV GC="-" SET GL=GC QUIT
 IF C="(" SET:'SV GC="(" SET GL=GC QUIT
 IF C=")" SET:'SV GC=")" SET GL=GC QUIT
 DO:VP
 . IF C="c" DO  QUIT
 . . SET:$GET(CP)="" CP=$LENGTH(CS)
 . . SET GC=$EXTRACT(CS,CP),CP=CP-1
 . . SET:CP<1 SP=$LENGTH(CS)
 . . QUIT
 . IF "fn+-"[C SET GC=$EXTRACT(V1,VP),VP=VP-1 QUIT
 . IF C="s" DO  QUIT
 . . SET:$GET(SP)="" SP=$LENGTH(SC)
 . . SET GC=$EXTRACT(SC,SP),SP=SP-1
 . . SET:SP<1 SP=$LENGTH(SC)
 . . QUIT
 . QUIT
 QUIT
 ;
 ; "c"    - Currency
 ; "f"    - Floating
 ; "n"    - Numeric
 ; "s"    - Separator
 ; "+-()" - Sign Representations
 ;
%TRANSV2 ; Translate the value into the mask
 SET GC=" "
 QUIT:"x "[C
 ;
 DO:VP
 . IF "f"[C DO  QUIT
 . . SET:$GET(CP)="" CP=1
 . . SET GC=$EXTRACT(CS,CP),CP=CP+1
 . . SET:CP>$LENGTH(CS) CP=1
 . . QUIT
 . IF C="n" DO  QUIT
 . . SET GC=$EXTRACT(V2,VP),VP=VP+1
 . . SET:VP>$LENGTH(V2) VP=0
 . . QUIT
 . IF C="s" DO  QUIT
 . . SET GC=$EXTRACT(SC,SP),SP=SP+1
 . . SET:SP<$LENGTH(SP) SP=1
 . . QUIT
 . QUIT
 IF "c"[C DO  QUIT
 . SET:$GET(CP)="" CP=1
 . SET GC=$EXTRACT(CS,CP)
 . SET:CP>$LENGTH(CS) GC=" "
 . SET CP=CP+1
 . QUIT
 IF C="+" SET GC="+" SET:'SV GC="-" SET GL=GC QUIT
 IF C="-" SET:'SV GC="-" SET GL=GC QUIT
 IF C="(" SET:'SV GC="(" SET GL=GC QUIT
 IF C=")" SET:'SV GC=")" SET GL=GC QUIT
 QUIT
 ;
%ERREX ; Error Exit point
 DO %ERROR
 QUIT K
 ;
 ; EC  - Error Coded String (1 character or longer)
 ; EL  - Error Code Length
 ; FL  - Field Length
 ; K   - Output String, The Error Message goes here.
%ERROR ; %ERROR HANDLING
 NEW C,E,EL,L
 SET:$GET(FL)<1 FL=$$%FLDLNG(0)
 SET E=$GET(EC),K="",L=1
 SET:E="" E="*"
 SET EL=$LENGTH(E)
 FOR I=1:1:FL SET C=$EXTRACT(E,L),L=L+1 SET:L>EL L=1 SET K=K_C
 QUIT
 ;
%LOADTV ; Do the translation of the temporary value with the string
 NEW X
 SET X=""
 FOR M=1:2:$LENGTH(TV,"%%") DO
 . SET X=X_$PIECE(TV,"%%",M)
 . SET N=$PIECE(TV,"%%",M+1)
 . SET:N X=X_ST(N)
 . QUIT
 SET TV=X
 QUIT
 ;
%FLDLNG(F) ; FIELD LENGTH Callable from Just About Anywhere
 SET F=$GET(F)
 QUIT:F F
 ;
 SET F=$LENGTH($GET(FO("FM")))
 IF 'F DO
 . SET F=$GET(FO("FL"))
 . IF 'F DO
 . . SET F=$LENGTH($GET(^$FORMAT("FM")))
 . . IF 'F DO
 . . . SET F=$GET(^$FORMAT("FL"))
 . . . IF 'F DO
 . . . . SET F=$LENGTH($GET(^$SYSTEM("FORMAT","FM")))
 . . . . IF 'F SET F=$GET(^$SYSTEM("FORMAT","FL"))
 . . . . QUIT
 . . . QUIT
 . . QUIT
 . QUIT 
 SET:'F F=10
 QUIT F
 ;
 ; Format Default Load
 ;
%INFORM ; Load up the defaults
 NEW K,X
 SET K="",X="FORMAT"
 IF '$DATA(^$FORMAT) DO  QUIT:$DATA(^$SYSTEM(X))
 . IF '$DATA(^$SYSTEM(X)) DO  QUIT
 . . SET ^$FORMAT("SC")=",",^$FORMAT("DC")="."
 . . SET ^$FORMAT("CS")="$",^$FORMAT("EC")="*"
 . . QUIT
 . MERGE ^$FORMAT=^$SYSTEM("FORMAT")
 . QUIT
 ; IF ^SYSTEM DOES NOT EXIST, CREATE IT
 DO:'$DATA(^$SYSTEM(X))
 . MERGE ^$SYSTEM("FORMAT")=^$FORMAT
 . QUIT
 QUIT
 ;
 ;===
 ;
 ;
CRC16(string,seed) ;
 ;
 ; The code below was approved in document X11/1998-32
 ;
 ; Polynomial x**16 + x**15 + x**2 + x**0
 NEW I,J,R
 IF '$DATA(seed) SET R=0
 ELSE  IF seed'<0,seed'>65535 SET R=seed\1
 ELSE  SET $ECODE=",M28,"
 FOR I=1:1:$LENGTH(string) DO
 . SET R=$$%XOR($ASCII(string,I),R,8)
 . FOR J=0:1:7 DO
 . . IF R#2 SET R=$$%XOR(R\2,40961,16)
 . . ELSE  SET R=R\2
 . . QUIT
 . QUIT
 QUIT R
%XOR(a,b,w) NEW I,M,R
 SET R=b,M=1
 FOR I=1:1:w DO
 . SET:a\M#2 R=R+$SELECT(R\M#2:-M,1:M)
 . SET M=M+M
 . QUIT
 QUIT R
 ;===
 ;
 ;
CRC32(string,seed) ;
 ;
 ; The code below was approved in document X11/1998-32
 ;
 ; Polynomial X**32 + X**26 + X**23 + X**22 +
 ;          + X**16 + X**12 + X**11 + X**10 +
 ;          + X**8  + X**7  + X**5  + X**4 +
 ;          + X**2  + X     + 1
 NEW I,J,R
 IF '$DATA(seed) SET R=4294967295
 ELSE  IF seed'<0,seed'>4294967295 SET R=seed\1
 ELSE  SET $ECODE=",M28,"
 FOR I=1:1:$LENGTH(string) DO
 . SET R=$$%XOR($ASCII(string,I),R,8)
 . FOR J=0:1:7 DO
 . . IF R#2 SET R=$$%XOR(R\2,3988292384,32)
 . . ELSE  SET R=R\2
 . . QUIT
 . QUIT
 QUIT 4294967295-R
 ; ===
 ;
 ;
CRCCCITT(string,seed) ;
 ;
 ; The code below was approved in document X11/1998-32
 ;
 ; Polynomial x**16 + x**12 + x**5 + x**0
 NEW I,J,R
 IF '$DATA(seed) SET R=65535
 ELSE  IF seed'<0,seed'>65535 SET R=seed\1
 ELSE  SET $ECODE=",M28,"
 FOR I=1:1:$LENGTH(string) DO
 . SET R=$$%XOR($ASCII(string,I)*256,R,16)
 . FOR J=0:1:7 DO
 . . SET R=R+R
 . . QUIT:R<65536
 . . SET R=$$%XOR(4129,R-65536,13)
 . . QUIT
 . QUIT
 QUIT R
 ; ===
 ;
 ;
LOWER(A,CHARMOD) NEW lo,up,x,y
 ;
 ; The code below was approved in document X11/1998-21
 ;
 SET x=$GET(CHARMOD)
 SET lo="abcdefghijklmnopqrstuvwxyz"
 SET up="ABCDEFGHIJKLMNOPQRSTUVWXYZ"
 IF x?1"^"1E.E DO
 . SET x=$EXTRACT(x,2,$LENGTH(x))
 . IF x?1"|".E DO
 . . SET x=$REVERSE($EXTRACT(x,2,$LENGTH(x)))
 . . SET y=$REVERSE($PIECE(x,"|",2,$LENGTH(x)+2))
 . . SET x=$REVERSE($PIECE(x,"|",1))
 . . SET x=$GET(^|y|$GLOBAL(x,"CHARACTER"))
 . . QUIT
 . ELSE  SET x=$GET(^$GLOBAL(x,"CHARACTER"))
 . QUIT
 IF x="" SET x=^$JOB($JOB,"CHARACTER")
 SET x=$GET(^$CHARACTER(x,"LOWER"))
 IF x="" QUIT $TRANSLATE(A,up,lo)
 SET @("x="_x_"(A)")
 QUIT x
 ; ===
 ;
 ;
PATCODE(A,PAT,CHARMOD) NEW x,y
 ;
 ; The code below was approved in document X11/1998-21
 ;
 SET x=$GET(CHARMOD)
 IF x?1"^"1E.E DO
 . SET x=$EXTRACT(x,2,$LENGTH(x))
 . IF x?1"|".E DO
 . . SET x=$REVERSE($EXTRACT(x,2,$LENGTH(x)))
 . . SET y=$REVERSE($PIECE(x,"|",2,$LENGTH(x)+2))
 . . SET x=$REVERSE($PIECE(x,"|",1))
 . . SET x=$GET(^|y|$GLOBAL(x,"CHARACTER"))
 . . QUIT
 . ELSE  SET x=$GET(^$GLOBAL(x,"CHARACTER"))
 . QUIT
 IF x="" SET x=^$JOB($JOB,"CHARACTER")
 SET x=$GET(^$CHARACTER(x,"PATCODE",PAT))
 IF x="" QUIT 0
 SET @("x="_x_"(A)")
 QUIT x
 ; ===
 ;
 ;
UPPER(A,CHARMOD) NEW lo,up,x,y
 ;
 ; The code below was approved in document X11/1998-21
 ;
 SET x=$GET(CHARMOD)
 SET lo="abcdefghijklmnopqrstuvwxyz"
 SET up="ABCDEFGHIJKLMNOPQRSTUVWXYZ"
 IF x?1"^"1E.E DO
 . SET x=$EXTRACT(x,2,$LENGTH(x))
 . IF x?1"|".E DO
 . . SET x=$REVERSE($EXTRACT(x,2,$LENGTH(x)))
 . . SET y=$REVERSE($PIECE(x,"|",2,$LENGTH(x)+2))
 . . SET x=$REVERSE($PIECE(x,"|",1))
 . . SET x=$GET(^|y|$GLOBAL(x,"CHARACTER"))
 . . QUIT
 . ELSE  SET x=$GET(^$GLOBAL(x,"CHARACTER"))
 . QUIT
 IF x="" SET x=^$JOB($JOB,"CHARACTER")
 SET x=$GET(^$CHARACTER(x,"UPPER"))
 IF x="" QUIT $TRANSLATE(A,lo,up)
 SET @("x="_x_"(A)")
 QUIT x
 ; ===
 ;
 ;

