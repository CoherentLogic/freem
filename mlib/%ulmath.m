%ulmath ; MATH library - version 0.5.0.1
 ;
 ; Unless otherwise noted, the code below
 ; was approved in document X11/95-11
 ;
 ; CORRECTION INFORMATION
 ; Due to the overall routine size this has not been provided
 ; It is available from Ed de Moel who provided these functions
 ;
ABS(X) Quit $Translate(+X,"-")
 ;===
 ;
 ;
%ARCCOS(X) ;
 ;
 New A,N,R,SIGN,XX
 If X<-1 Set $Ecode=",M28,"
 If X>1 Set $Ecode=",M28,"
 Set SIGN=1 Set:X<0 X=-X,SIGN=-1
 Set A(0)=1.5707963050,A(1)=-0.2145988016,A(2)=0.0889789874
 Set A(3)=-0.0501743046,A(4)=0.0308918810,A(5)=-0.0170881256
 Set A(6)=0.0066700901,A(7)=-0.0012624911
 Set R=A(0),XX=1 For N=1:1:7 Set XX=XX*X,R=A(N)*XX+R
 ;
 Set R=$$SQRT(1-X,11)*R
 ;;;
 ;
 Quit R*SIGN
 ;===
 ;
 ;
ARCCOS(X,PREC) ;
 I '$D(PREC) Q $$%ARCCOS(X)
 ;
 New L,LIM,K,SIG,SIGS,VALUE
 ;;;
 ;
 If X<-1 Set $Ecode=",M28,"
 If X>1 Set $Ecode=",M28,"
 Set PREC=$Get(PREC,11)
 ;
 If $Translate(X,"-")=1 Quit 0
 ;;;
 ;
 Set SIG=$Select(X<0:-1,1:1),VALUE=1-(X*X)
 ;
 Set X=$$SQRT(VALUE,PREC)
 ;;;
 ;
 If $Translate(X,"-")=1 Do  Quit VALUE
 . ;;;
 . ;
 . Set VALUE=$$PI()/2*X
 . Quit
 ;
 If X>0.9 Do  Quit VALUE
 . ;;;
 . ;
 . Set SIGS=$Select(X<0:-1,1:1)
 . Set VALUE=1/(1/X/X-1)
 . ;
 . Set X=$$SQRT(VALUE,PREC)
 . ;;;
 . ;
 . ;
 . Set VALUE=$$ARCTAN(X,PREC)*SIGS
 . ;;;
 ;
 . Quit
 Set (VALUE,L)=X
 Set LIM=$Select((PREC+3)'>11:PREC+3,1:11),@("LIM=1E-"_LIM)
 For K=3:2 Do  Quit:($Translate(L,"-")<LIM)
 . Set L=L*X*X*(K-2)/(K-1)*(K-2)/K,VALUE=VALUE+L
 . Quit
 Quit $Select(SIG<0:$$PI()-VALUE,1:VALUE)
 ;===
 ;
 ;
ARCCOSH(X,PREC) ;
 If X<1 Set $Ecode=",M28,"
 New SQ
 ;
 Set PREC=$Get(PREC,11)
 ;;;
 ;
 Set SQ=$$SQRT(X*X-1,PREC)
 Quit $$LOG(X+SQ,PREC)
 ;===
 ;
 ;
ARCCOT(X,PREC) ;
 Set PREC=$Get(PREC,11)
 Set X=1/X
 Quit $$ARCTAN(X,PREC)
 ;===
 ;
 ;
ARCCOTH(X,PREC) ;
 New L1,L2
 ;
 Set PREC=$Get(PREC,11)
 ;;;
 ;
 Set L1=$$LOG(X+1,PREC)
 Set L2=$$LOG(X-1,PREC)
 Quit L1-L2/2
 ;===
 ;
 ;
ARCCSC(X,PREC) ;
 Set PREC=$Get(PREC,11)
 Set X=1/X
 Quit $$ARCSIN(X,PREC)
 ;===
 ;
 ;
ARCSEC(X,PREC) ;
 Set PREC=$Get(PREC,11)
 Set X=1/X
 Quit $$ARCCOS(X,PREC)
 ;===
 ;
 ;
%ARCSIN(X) ;
 ;;; ;                                                                 Number ~~
 ; Winfried Gerum (8 June 1995)
 ;
 New A,N,R,SIGN,XX
 If X<-1 Set $Ecode=",M28,"
 If X>1 Set $Ecode=",M28,"
 Set SIGN=1 Set:X<0 X=-X,SIGN=-1
 Set A(0)=1.5707963050,A(1)=-0.2145988016,A(2)=0.0889789874
 Set A(3)=-0.0501743046,A(4)=0.0308918810,A(5)=-0.0170881256
 Set A(6)=0.0066700901,A(7)=-0.0012624911
 Set R=A(0),XX=1 For N=1:1:7 Set XX=XX*X,R=A(N)*XX+R
 ;
 Set R=$$SQRT(1-X,11)*R
 ;;;
 ;
 Set R=$$PI()/2-R
 Quit R*SIGN
 ;===
 ;
 ;
ARCSIN(X,PREC) ;
 I '$D(PREC) Q $$%ARCSIN(X)
 New L,LIM,K,SIGS,VALUE
 Set PREC=$Get(PREC,11)
 ;
 If $Translate(X,"-")=1 Do  Quit VALUE
 . ;;;
 . ;
 . Set VALUE=$$PI()/2*X
 . Quit
 ;
 If X>0.99999 Do  Quit VALUE
 . ;;;
 . ;
 . Set SIGS=$Select(X<0:-1,1:1)
 . Set VALUE=1/(1/X/X-1)
 . ;
 . ;;;
 . ;
 . Set VALUE=$$ARCTAN(X,PREC)*SIGS
 . ;;;
 . ;
 . Quit
 Set (VALUE,L)=X
 Set LIM=$Select((PREC+3)'>11:PREC+3,1:11),@("LIM=1E-"_LIM)
 For K=3:2 Do  Quit:($Translate(L,"-")<LIM)
 . Set L=L*X*X*(K-2)/(K-1)*(K-2)/K,VALUE=VALUE+L
 . Quit
 Quit VALUE
 ;===
 ;
 ;
ARCSINH(X,PREC) ;
 If X<1 Set $Ecode=",M28,"
 New SQ
 ;
 Set PREC=$Get(PREC,11)
 ;;;
 ;
 Set SQ=$$SQRT(X*X+1,PREC)
 Quit $$LOG(X+SQ,PREC)
 ;===
 ;
 ;
ARCTAN(X,PREC) ;
 New FOLD,HI,L,LIM,LO,K,SIGN,SIGS,SIGT,VALUE
 Set PREC=$Get(PREC,11)
 Set LO=0.0000000001,HI=9999999999
 Set SIGT=$Select(X<0:-1,1:1),X=$Translate(X,"-")
 Set X=$Select(X<LO:LO,X>HI:HI,1:X)
 ;
 Set FOLD=$Select(X'<1:0,1:1)
 ;;;
 ;
 Set X=$Select(FOLD:1/X,1:X)
 Set L=X,VALUE=$$PI()/2-(1/X),SIGN=1
 ;
 If X<1.3 Do  Quit VALUE
 . ;;;
 . ;
 . Set X=$Select(FOLD:1/X,1:X),VALUE=1/((1/X/X)+1)
 . ;
 . Set X=$$SQRT(VALUE,PREC)
 . ;;;
 . ;
 . If $Translate(X,"-")=1 Do  Quit
 . . Set VALUE=$$PI()/2*X
 . . Quit
 . If X>0.9 Do  Quit
 . . Set SIGS=$Select(X<0:-1,1:1)
 . . Set VALUE=1/(1/X/X-1)
 . . Set X=$$SQRT(VALUE)
 . . Set VALUE=$$ARCTAN(X,10)
 . . Set VALUE=VALUE*SIGS
 . . Quit
 . Set (VALUE,L)=X
 . Set LIM=$Select((PREC+3)'>11:PREC+3,1:11),@("LIM=1E-"_LIM)
 . For K=3:2 Do  Quit:($Translate(L,"-")<LIM)
 . . Set L=L*X*X*(K-2)/(K-1)*(K-2)/K,VALUE=VALUE+L
 . . Quit
 . Set VALUE=$Select(SIGT<1:-VALUE,1:VALUE)
 . Quit
 Set LIM=$Select((PREC+3)'>11:PREC+3,1:11),@("LIM=1E-"_LIM)
 For K=3:2 Do  Quit:($Translate(1/L,"-")<LIM)
 . ;
 . Set L=L*X*X,VALUE=VALUE+(1/(K*L)*SIGN)
 . ;;;
 . ;
 . Set SIGN=SIGN*-1
 . Quit
 Set VALUE=$Select(FOLD:$$PI()/2-VALUE,1:VALUE)
 Set VALUE=$Select(SIGT<1:-VALUE,1:VALUE)
 Quit VALUE
 ;===
 ;
 ;
ARCTANH(X,PREC) ;
 If X<-1 Set $Ecode=",M28,"
 If X>1 Set $Ecode=",M28,"
 ;
 Set PREC=$Get(PREC,11)
 ;;;
 ;
 Quit $$LOG(1+X/(1-X),PREC)/2
 ;===
 ;
 ;
CABS(Z) ;
 New ZRE,ZIM
 Set ZRE=+Z,ZIM=+$Piece(Z,"%",2)
 Quit $$SQRT(ZRE*ZRE+(ZIM*ZIM))
 ;===
 ;
 ;
CADD(X,Y) ;
 New XRE,XIM,YRE,YIM
 Set XRE=+X,XIM=+$Piece(X,"%",2)
 Set YRE=+Y,YIM=+$Piece(Y,"%",2)
 Quit XRE+YRE_"%"_(XIM+YIM)
 ;===
 ;
 ;
CCOS(Z,PREC) ;
 New E1,E2,IA
 ;
 Set PREC=$Get(PREC,11)
 ;;;
 ;
 Set IA=$$CMUL(Z,"0%1")
 Set E1=$$CEXP(IA,PREC)
 Set IA=-IA_"%"_(-$Piece(IA,"%",2))
 Set E2=$$CEXP(IA,PREC)
 Set IA=$$CADD(E1,E2)
 Quit $$CMUL(IA,"0.5%0")
 ;===
 ;
 ;
CDIV(X,Y) ;
 New D,IM,RE,XIM,XRE,YIM,YRE
 Set XRE=+X,XIM=+$Piece(X,"%",2)
 Set YRE=+Y,YIM=+$Piece(Y,"%",2)
 Set D=YRE*YRE+(YIM*YIM)
 Set RE=XRE*YRE+(XIM*YIM)/D
 Set IM=XIM*YRE-(XRE*YIM)/D
 Quit RE_"%"_IM
 ;===
 ;
 ;
CEXP(Z,PREC) ;
 New R,ZIM,ZRE
 ;
 Set PREC=$Get(PREC,11)
 ;;;
 ;
 Set ZRE=+Z,ZIM=+$Piece(Z,"%",2)
 Set R=$$EXP(ZRE,PREC)
 Quit R*$$COS(ZIM,PREC)_"%"_(R*$$SIN(ZIM,PREC))
 ;===
 ;
 ;
CLOG(Z,PREC) ;
 New ABS,ARG,ZIM,ZRE
 ;
 Set PREC=$Get(PREC,11)
 ;;;
 ;
 Set ABS=$$CABS(Z)
 Set ZRE=+Z,ZIM=+$Piece(Z,"%",2)
 ;
 Set ARG=$$ARCTAN(ZIM/ZRE,PREC)
 ;;;
 ;
 Quit $$LOG(ABS,PREC)_"%"_ARG
 ;===
 ;
 ;
CMUL(X,Y) ;
 New XIM,XRE,YIM,YRE
 Set XRE=+X,XIM=+$Piece(X,"%",2)
 Set YRE=+Y,YIM=+$Piece(Y,"%",2)
 Quit XRE*YRE-(XIM*YIM)_"%"_(XRE*YIM+(XIM*YRE))
 ;===
 ;
 ;
COMPLEX(X) Quit +X_"%0"
 ;===
 ;
 ;
CONJUG(Z) ;
 New ZIM,ZRE
 Set ZRE=+Z,ZIM=+$Piece(Z,"%",2)
 Quit ZRE_"%"_(-ZIM)
 ;===
 ;
 ;
COS(X,PREC) ;
 I '$D(PREC) Q $$%COS(X)
 New L,LIM,K,SIGN,VALUE
 ;
 Set:X[":" X=$$DMSDEC(X)
 ;;;
 ;
 Set PREC=$Get(PREC,11)
 Set X=X#(2*$$PI())
 Set (VALUE,L)=1,SIGN=-1
 Set LIM=$Select((PREC+3)'>11:PREC+3,1:11),@("LIM=1E-"_LIM)
 For K=2:2 Do  Quit:($Translate(L,"-")<LIM)  Set SIGN=SIGN*-1
 . Set L=L*X*X/(K-1*K),VALUE=VALUE+(SIGN*L)
 . Quit
 Quit VALUE
 ;===
 ;
 ;
%COS(X) ;
 ;;; ;                                                                 Number ~~
 ; Winfried Gerum (8 June 1995)
 ;
 New A,N,PI,R,SIGN,XX
 ;
 ; This approximation only works for 0 <= x <= pi/2
 ; so reduce angle to correct quadrant.
 ;
 Set PI=$$PI(),X=X#(PI*2),SIGN=1
 Set:X>PI X=2*PI-X
 Set:X*2>PI X=PI-X,SIGN=-1
 ;
 Set XX=X*X,A(1)=-0.4999999963,A(2)=0.0416666418
 Set A(3)=-0.0013888397,A(4)=0.0000247609,A(5)=-0.0000002605
 Set (X,R)=1 For N=1:1:5 Set X=X*XX,R=A(N)*X+R
 Quit R*SIGN
 ;===
 ;
 ;
COSH(X,PREC) ;
 ;
 New E,F,I,P,R,T,XX
 ;;;
 ;
 Set PREC=$Get(PREC,11)+1
 Set @("E=1E-"_PREC)
 Set XX=X*X,F=1,(P,R,T)=1,I=1
 For  Set T=T*XX,F=I+1*I*F,R=T/F+R,P=P-R/R,I=I+2 If -E<P,P<E Quit
 Quit R
 ;===
 ;
 ;
COT(X,PREC) ;
 New C,L,LIM,K,SIGN,VALUE
 Set:X[":" X=$$DMSDEC(X)
 ;;;
 ;
 Set PREC=$Get(PREC,11)
 Set (VALUE,L)=1,SIGN=-1
 Set LIM=$Select((PREC+3)'>11:PREC+3,1:11),@("LIM=1E-"_LIM)
 For K=2:2 Do  Quit:($Translate(L,"-")<LIM)  Set SIGN=SIGN*-1
 . Set L=L*X*X/(K-1*K),VALUE=VALUE+(SIGN*L)
 . Quit
 Set C=VALUE
 Set X=X#(2*$$PI())
 Set (VALUE,L)=X,SIGN=-1
 Set LIM=$Select((PREC+3)'>11:PREC+3,1:11),@("LIM=1E-"_LIM)
 For K=3:2 Do  Quit:($Translate(L,"-")<LIM)  Set SIGN=SIGN*-1
 . Set L=L/(K-1)*X/K*X,VALUE=VALUE+(SIGN*L)
 . Quit
 If 'VALUE Quit "INFINITE"
 Quit VALUE=C/VALUE
 ;===
 ;
 ;
COTH(X,PREC) ;
 New SINH
 If 'X Quit "INFINITE"
 ;
 Set PREC=$Get(PREC,11)
 ;;;
 ;
 Set SINH=$$SINH(X,PREC)
 If 'SINH Quit "INFINITE"
 Quit $$COSH(X,PREC)/SINH
 ;===
 ;
 ;
CPOWER(Z,N,PREC) ;
 New AR,NIM,NRE,PHI,PI,R,RHO,TH,ZIM,ZRE
 ;
 Set PREC=$Get(PREC,11)
 ;;;
 ;
 Set ZRE=+Z,ZIM=+$Piece(Z,"%",2)
 Set NRE=+N,NIM=+$Piece(N,"%",2)
 If 'ZRE,'ZIM,'NRE,'NIM Set $Ecode=",M28,"
 ;
 If 'ZRE,'ZIM Quit "0%0"
 ;;;
 ;
 Set PI=$$PI()
 ;
 Set R=$$SQRT(ZRE*ZRE+(ZIM*ZIM),PREC)
 ;;;
 ;
 ;
 If ZRE Set TH=$$ARCTAN(ZIM/ZRE,PREC)
 ;;;
 ;
 Else  Set TH=$SELECT(ZIM>0:PI/2,1:-PI/2)
 ;;;
 ;
 Set RHO=$$LOG(R,PREC)
 Set AR=$$EXP(RHO*NRE-(TH*NIM),PREC)
 Set PHI=RHO*NIM+(NRE*TH)
 Quit AR*$$COS(PHI,PREC)_"%"_(AR*$$SIN(PHI,PREC))
 ;===
 ;
 ;
CSC(X,PREC) ;
 New L,LIM,K,SIGN,VALUE
 ;
 ; Winfried Gerum (8 June 1995)
 Set:X[":" X=$$DMSDEC(X)
 ;;;
 ;
 Set PREC=$Get(PREC,11)
 ;;;
 ;
 Set X=X#(2*$$PI())
 Set (VALUE,L)=X,SIGN=-1
 Set LIM=$Select((PREC+3)'>11:PREC+3,1:11),@("LIM=1E-"_LIM)
 For K=3:2 Do  Quit:($Translate(L,"-")<LIM)  Set SIGN=SIGN*-1
 . Set L=L/(K-1)*X/K*X,VALUE=VALUE+(SIGN*L)
 . Quit
 If 'VALUE Quit "INFINITE"
 Quit 1/VALUE
 ;===
 ;
 ;
 ;
CSCH(X,PREC) 
 Quit 1/$$SINH(X,$Get(PREC,11))
 ;;;
 ;
 ;===
 ;
 ;
CSIN(Z,PREC) ;
 New IA,E1,E2
 ;
 Set PREC=$Get(PREC,11)
 ;;;
 ;
 Set IA=$$CMUL(Z,"0%1")
 Set E1=$$CEXP(IA,PREC)
 Set IA=-IA_"%"_(-$Piece(IA,"%",2))
 Set E2=$$CEXP(IA,PREC)
 Set IA=$$CSUB(E1,E2)
 Set IA=$$CMUL(IA,"0.5%0")
 Quit $$CMUL("0%-1",IA)
 ;===
 ;
 ;
CSUB(X,Y) ;
 New XIM,XRE,YIM,YRE
 Set XRE=+X,XIM=+$Piece(X,"%",2)
 Set YRE=+Y,YIM=+$Piece(Y,"%",2)
 Quit XRE-YRE_"%"_(XIM-YIM)
 ;===
 ;
 ;
DECDMS(X,PREC) ;
 Set PREC=$Get(PREC,5)
 Set X=X#360*3600
 Set X=+$Justify(X,0,$Select((PREC-$Length(X\1))'<0:PREC-$Length(X\1),1:0))
 Quit X\3600_":"_(X\60#60)_":"_(X#60)
 ;===
 ;
 ;
DEGRAD(X) Quit X*3.14159265358979/180
 ;===
 ;
 ;
DMSDEC(X) ;
 Quit $Piece(X,":")+($Piece(X,":",2)/60)+($Piece(X,":",3)/3600)
 ;===
 ;
 ;
E() Quit 2.71828182845905
 ;===
 ;
 ;
EXP(X,PREC) ;
 New L,LIM,K,VALUE
 Set PREC=$Get(PREC,11)
 Set L=X,VALUE=X+1
 Set LIM=$Select((PREC+3)'>11:PREC+3,1:11),@("LIM=1E-"_LIM)
 For K=2:1 Set L=L*X/K,VALUE=VALUE+L Quit:($Translate(L,"-")<LIM)
 Quit VALUE
 ;===
 ;
 ;
LOG(X,PREC) ;
 New L,LIM,M,N,K,VALUE
 If X'>0 Set $Ecode=",M28,"
 Set PREC=$Get(PREC,11)
 Set M=1
 ;
 For N=0:1 Quit:(X/M)<10  Set M=M*10
 ;;;
 ;
 If X<1 For N=0:-1 Quit:(X/M)>0.1  Set M=M*0.1
 Set X=X/M
 Set X=(X-1)/(X+1),(VALUE,L)=X
 Set LIM=$Select((PREC+3)'>11:PREC+3,1:11),@("LIM=1E-"_LIM)
 For K=3:2 Set L=L*X*X,M=L/K,VALUE=M+VALUE Set:M<0 M=-M Quit:M<LIM
 Set VALUE=VALUE*2+(N*2.30258509298749)
 Quit VALUE
 ;===
 ;
 ;
LOG10(X,PREC) ;
 New L,LIM,M,N,K,VALUE
 If X'>0 Set $Ecode=",M28,"
 Set PREC=$Get(PREC,11)
 Set M=1
 ;
 For N=0:1 Quit:(X/M)<10  Set M=M*10
 ;;;
 ;
 If X<1 For N=0:-1 Quit:(X/M)>0.1  Set M=M*0.1
 Set X=X/M
 Set X=(X-1)/(X+1),(VALUE,L)=X
 Set LIM=$Select((PREC+3)'>11:PREC+3,1:11),@("LIM=1E-"_LIM)
 For K=3:2 Set L=L*X*X,M=L/K,VALUE=M+VALUE Set:M<0 M=-M Quit:M<LIM
 Set VALUE=VALUE*2+(N*2.30258509298749)
 Quit VALUE/2.30258509298749
 ;===
 ;
 ;
MTXADD(A,B,R,ROWS,COLS) ;
 ; Add A[ROWS,COLS] to B[ROWS,COLS],
 ; result goes to R[ROWS,COLS]
 IF $DATA(A)<10 QUIT 0
 IF $DATA(B)<10 QUIT 0
 IF $GET(ROWS)<1 QUIT 0
 IF $GET(COLS)<1 QUIT 0
 ;
 NEW ROW,COL,ANY
 FOR ROW=1:1:ROWS FOR COL=1:1:COLS DO
 . KVALUE R(ROW,COL) SET ANY=0
 . SET:$DATA(A(ROW,COL))#2 ANY=1
 . SET:$DATA(B(ROW,COL))#2 ANY=1
 . SET:ANY R(ROW,COL)=$GET(A(ROW,COL))+$GET(B(ROW,COL))
 . QUIT
 QUIT 1
 ;===
 ;
 ;
MTXCOF(A,I,K,N) ;
 ; Compute cofactor for element [i,k]
 ; in matrix A[N,N]
 NEW T,R,C,RR,CC
 SET CC=0 FOR C=1:1:N DO:C'=K
 . SET CC=CC+1,RR=0
 . FOR R=1:1:N SET:R'=I RR=RR+1,T(RR,CC)=$GET(A(R,C))
 . QUIT
 QUIT $$MTXDET(.T,N-1)
 ;===
 ;
 ;
MTXCOPY(A,R,ROWS,COLS) ;
 ; Copy A[ROWS,COLS] to R[ROWS,COLS]
 IF $DATA(A)<10 QUIT 0
 IF $GET(ROWS)<1 QUIT 0
 IF $GET(COLS)<1 QUIT 0
 ;
 NEW ROW,COL
 FOR ROW=1:1:ROWS FOR COL=1:1:COLS DO
 . KVALUE R(ROW,COL)
 . SET:$DATA(A(ROW,COL))#2 R(ROW,COL)=A(ROW,COL)
 . QUIT
 QUIT 1
 ;===
 ;
 ;
MTXDET(A,N) ;
 ; Compute determinant of matrix A[N,N]
 IF $DATA(A)<10 QUIT ""
 IF N<1 QUIT ""
 ;
 ; First the simple cases
 ;
 IF N=1 QUIT $GET(A(1,1))
 IF N=2 QUIT $GET(A(1,1))*$GET(A(2,2))-($GET(A(1,2))*$GET(A(2,1)))
 ;
 NEW DET,I,SIGN
 ;
 ; Det A = sum (k=1:n) element (i,k) * cofactor [i,k]
 ;
 SET DET=0,SIGN=1
 FOR I=1:1:N DO
 . SET DET=$GET(A(1,I))*$$MTXCOF(.A,1,I,N)*SIGN+DET
 . SET SIGN=-SIGN
 . QUIT
 QUIT DET
 ;===
 ;
 ;
MTXEQU(A,B,R,N,M) ;
 ; Solve matrix equation A [M,M] * R [M,N] = B [M,N]
 IF $GET(M)<1 QUIT ""
 IF $GET(N)<1 QUIT ""
 IF '$$MTXDET(.A) QUIT 0
 ;
 NEW I,I1,J,J1,J2,K,L,T,T1,T2,TEMP,X
 ;
 SET X=$$MTXCOPY(.A,.T,N,N)
 SET X=$$MTXCOPY(.B,.R,N,M)
 ;
 ; Reduction of matrix A
 ; Steps of reduction are counted by index K
 ;
 FOR K=1:1:N-1 DO
 . ;
 . ; Search for largest coefficient of T
 . ; (denoted by TEMP)
 . ; in first column of reduced system
 . ;
 . SET TEMP=0,J2=K
 . FOR J1=K:1:N DO
 . . QUIT:$TRANSLATE($GET(T(J1,K)),"-")>$TRANSLATE(TEMP,"-")
 . . SET TEMP=T(J1,K),J2=J1
 . . QUIT
 . ;
 . ; Exchange row number K with row number J2,
 . ; if necessary
 . ;
 . DO:J2'=K
 . . ;
 . . FOR J=K:1:N DO
 . . . SET T1=$GET(T(K,J)),T2=$GET(T(J2,J))
 . . . KILL T(K,J),T(J2,J)
 . . . IF T1'="" SET T(J2,J)=T1
 . . . IF T2'="" SET T(K,J)=T2
 . . . QUIT
 . . FOR J=1:1:M DO
 . . . SET T1=$GET(R(K,J)),T2=$GET(R(J2,J))
 . . . KILL R(K,J),R(J2,J)
 . . . IF T1'="" SET R(J2,J)=T1
 . . . IF T2'="" SET R(K,J)=T2
 . . . QUIT
 . . QUIT
 . ;
 . ; Actual reduction
 . ;
 . FOR I=K+1:1:N DO
 . . FOR J=K+1:1:N DO
 . . . QUIT:'$GET(T(K,K))
 . . . SET T(I,J)=-$GET(T(K,J))*$GET(T(I,K))/T(K,K)+$GET(T(I,J))
 . . . QUIT
 . . FOR J=1:1:M DO
 . . . QUIT:'$GET(T(K,K))
 . . . SET R(I,J)=-$GET(R(K,J))*$GET(T(I,K))/T(K,K)+$GET(R(I,J))
 . . . QUIT
 . . QUIT
 . QUIT
 ;
 ; Backsubstitution
 ;
 FOR J=1:1:M DO
 . IF $GET(T(N,N)) SET R(N,J)=$GET(R(N,J))/T(N,N)
 . IF N-1>0 FOR I1=1:1:N-1 DO
 . . SET I=N-I1
 . . FOR L=I+1:1:N DO
 . . . SET R(I,J)=-$GET(T(I,L))*$GET(R(L,J))+$GET(R(I,J))
 . . . QUIT
 . . IF $GET(T(I,I)) SET R(I,J)=$GET(R(I,J))/$GET(T(I,I))
 . . QUIT
 . QUIT
 QUIT $$MTXDET(.R)
 ;===
 ;
 ;
MTXINV(A,R,N) ;
 ; Invert A[N,N], result goes to R[N,N]
 IF $DATA(A)<10 QUIT 0
 IF $GET(N)<1 QUIT 0
 ;
 NEW T,X
 SET X=$$MTXUNIT(.T,N)
 QUIT $$MTXEQU(.A,.T,.R,N,N)
 ;===
 ;
 ;
MTXMUL(A,B,R,M,L,N) ;
 ; Multiply A[M,L] by B[L,N], result goes to R[M,N]
 IF $DATA(A)<10 QUIT 0
 IF $DATA(B)<10 QUIT 0
 IF $GET(L)<1 QUIT 0
 IF $GET(M)<1 QUIT 0
 IF $GET(N)<1 QUIT 0
 ;
 NEW I,J,K,SUM,ANY
 FOR I=1:1:M FOR J=1:1:N DO
 . SET (SUM,ANY)=0
 . KVALUE R(I,J)
 . FOR K=1:1:L DO
 . . SET:$DATA(A(I,K))#2 ANY=1
 . . SET:$DATA(B(K,J))#2 ANY=1
 . . SET SUM=$GET(A(I,K))*$GET(B(K,J))+SUM
 . . QUIT
 . SET:ANY R(I,J)=SUM
 . QUIT
 QUIT 1
 ;===
 ;
 ;
MTXSCA(A,R,ROWS,COLS,S) ;
 ; Multiply A[ROWS,COLS] with the scalar S,
 ; result goes to R[ROWS,COLS]
 IF $DATA(A)<10 QUIT 0
 IF $GET(ROWS)<1 QUIT 0
 IF $GET(COLS)<1 QUIT 0
 IF '($DATA(S)#2) QUIT 0
 ;
 NEW ROW,COL
 FOR ROW=1:1:ROWS FOR COL=1:1:COLS DO
 . KVALUE R(ROW,COL)
 . SET:$DATA(A(ROW,COL))#2 R(ROW,COL)=A(ROW,COL)*S
 . QUIT
 QUIT 1
 ;===
 ;
 ;
MTXSUB(A,B,R,ROWS,COLS) ;
 ; Subtract B[ROWS,COLS] from A[ROWS,COLS],
 ; result goes to R[ROWS,COLS]
 IF $DATA(A)<10 QUIT 0
 IF $DATA(B)<10 QUIT 0
 IF $GET(ROWS)<1 QUIT 0
 IF $GET(COLS)<1 QUIT 0
 ;
 NEW ROW,COL,ANY
 FOR ROW=1:1:ROWS FOR COL=1:1:COLS DO
 . KVALUE R(ROW,COL) SET ANY=0
 . SET:$DATA(A(ROW,COL))#2 ANY=1
 . SET:$DATA(B(ROW,COL))#2 ANY=1
 . ;
 . SET:ANY R(ROW,COL)=$GET(A(ROW,COL))-$GET(B(ROW,COL))
 . ;;;
 . ;
 . QUIT
 QUIT 1
 ;===
 ;
 ;
MTXTRP(A,R,M,N) ;
 ; Transpose A[M,N], result goes to R[N,M]
 IF $DATA(A)<10 QUIT 0
 IF $GET(M)<1 QUIT 0
 IF $GET(N)<1 QUIT 0
 ;
 NEW I,J,K,D1,V1,D2,V2
 FOR I=1:1:M+N-1 FOR J=1:1:I+1\2 DO
 . SET K=I-J+1
 . IF K=J DO  QUIT
 . . SET V1=$GET(A(J,J)),D1=$DATA(A(J,J))#2
 . . IF J'>N,J'>M KVALUE R(J,J) SET:D1 R(J,J)=V1
 . . QUIT
 . ;
 . SET V1=$GET(A(K,J)),D1=$DATA(A(K,J))#2
 . SET V2=$GET(A(J,K)),D2=$DATA(A(J,K))#2
 . IF K'>M,J'>N KVALUE R(K,J) SET:D2 R(K,J)=V2
 . IF J'>M,K'>N KVALUE R(J,K) SET:D1 R(J,K)=V1
 . QUIT
 QUIT 1
 ;===
 ;
 ;
MTXUNIT(R,N,SPARSE) ;
 ; Create a unit matrix R[N,N]
 IF $GET(N)<1 QUIT 0
 ;
 NEW ROW,COL
 FOR ROW=1:1:N FOR COL=1:1:N DO
 . KVALUE R(ROW,COL)
 . IF $GET(SPARSE) QUIT:ROW'=COL
 . SET R(ROW,COL)=$SELECT(ROW=COL:1,1:0)
 . QUIT
 QUIT 1
 ;===
 ;
 ;
PI() Quit 3.14159265358979
 ;===
 ;
 ;
RADDEG(X) Quit X*180/3.14159265358979
 ;===
 ;
 ;
SEC(X,PREC) ;
 New L,LIM,K,SIGN,VALUE
 ;
 Set:X[":" X=$$DMSDEC(X)
 ;;;
 ;
 Set PREC=$Get(PREC,11)
 Set X=X#(2*$$PI())
 Set (VALUE,L)=1,SIGN=-1
 Set LIM=$Select((PREC+3)'>11:PREC+3,1:11),@("LIM=1E-"_LIM)
 For K=2:2 Do  Quit:($Translate(L,"-")<LIM)  Set SIGN=SIGN*-1
 . Set L=L*X*X/(K-1*K),VALUE=VALUE+(SIGN*L)
 . Quit
 If 'VALUE Quit "INFINITE"
 Quit 1/VALUE
 ;===
 ;
 ;
SECH(X,PREC) 
 Quit 1/$$COSH(X,$Get(PREC,11))
 ;;;
 ;===
 ;
 ;
SIGN(X) Quit $SELECT(X<0:-1,X>0:1,1:0)
 ;===
 ;
 ;
SIN(X,PREC) ;
 I '$D(PREC) Q $$%SIN(X)
 New L,LIM,K,SIGN,VALUE
 ;
 Set:X[":" X=$$DMSDEC(X)
 ;;;
 ;
 Set PREC=$Get(PREC,11)
 Set X=X#(2*$$PI())
 Set (VALUE,L)=X,SIGN=-1
 Set LIM=$Select((PREC+3)'>11:PREC+3,1:11),@("LIM=1E-"_LIM)
 For K=3:2 Do  Quit:($Translate(L,"-")<LIM)  Set SIGN=SIGN*-1
 . Set L=L/(K-1)*X/K*X,VALUE=VALUE+(SIGN*L)
 . Quit
 Quit VALUE
 ;===
 ;
 ;
%SIN(X) ;
 ;
 New A,N,PI,R,SIGN,XX
 ;
 ; This approximation only works for 0 <= x <= pi/2
 ; so reduce angle to correct quadrant.
 ;
 Set PI=$$PI(),X=X#(PI*2),SIGN=1
 Set:X>PI X=2*PI-X,SIGN=-1
 ;
 Set:X*2<PI X=PI-X
 ;;;
 ;
 ;
 Set XX=X*X,A(1)=-0.4999999963,A(2)=0.0416666418
 Set A(3)=-0.0013888397,A(4)=0.0000247609,A(5)=-0.0000002605
 Set (X,R)=1 For N=1:1:5 Set X=X*XX,R=A(N)*X+R
 Quit R*SIGN
 ;===
 ;
 ;
SINH(X,PREC) ;
 ;
 New E,F,I,P,R,T,XX
 ;;;
 ;
 Set PREC=$Get(PREC,11)+1
 Set @("E=1E-"_PREC)
 Set XX=X*X,F=1,I=2,(P,R,T)=X
 For  Set T=T*XX,F=I+1*I*F,R=T/F+R,P=P-R/R,I=I+2 If -E<P,P<E Quit
 Quit R
 ;===
 ;
 ;
SQRT(X,PREC) ;
 If X<0 Set $Ecode=",M28,"
 If X=0 Quit 0
 ;
 Set PREC=$Get(PREC,11)
 ;;;
 ;
 ;
 If X<1 Quit 1/$$SQRT(1/X,PREC)
 ;;;
 ;
 New P,R,E
 Set PREC=$Get(PREC,11)+1
 ;
 Set @("E=1E-"_PREC)
 ;;;
 ;
 Set R=X
 For  Set P=R,R=X/R+R/2,P=P-R/R If -E<P,P<E Quit
 Quit R
 ;===
 ;
 ;
TAN(X,PREC) ;
 New L,LIM,K,S,SIGN,VALUE
 ;
 Set:X[":" X=$$DMSDEC(X)
 ;;;
 ;
 Set PREC=$Get(PREC,11)
 Set X=X#(2*$$PI())
 Set (VALUE,L)=X,SIGN=-1
 Set LIM=$Select((PREC+3)'>11:PREC+3,1:11),@("LIM=1E-"_LIM)
 For K=3:2 Do  Quit:($Translate(L,"-")<LIM)  Set SIGN=SIGN*-1
 . Set L=L/(K-1)*X/K*X,VALUE=VALUE+(SIGN*L)
 . Quit
 Set S=VALUE
 Set X=X#(2*$$PI())
 Set (VALUE,L)=1,SIGN=-1
 Set LIM=$Select((PREC+3)'>11:PREC+3,1:11),@("LIM=1E-"_LIM)
 For K=2:2 Do  Quit:($Translate(L,"-")<LIM)  Set SIGN=SIGN*-1
 . Set L=L*X*X/(K-1*K),VALUE=VALUE+(SIGN*L)
 . Quit
 If 'VALUE Quit "INFINITE"
 Quit S/VALUE
 ;===
 ;
 ;
TANH(X,PREC) ;
 ;
 Set PREC=$Get(PREC,11)
 ;;;
 ;
 Quit $$SINH(X,PREC)/$$COSH(X,PREC)
 ;===

