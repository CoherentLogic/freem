BENCH	;DBB;12-JUN-81 1:25 AM;BENCHMARK STANDARD MUMPS
	K  S K=1,T=1,T(0)=0,N=40000 ; SET N TO MULTIPLE OF 500
	K ^TSX W !,"Test #    Name      msec/Pass     Without for",!!
	F K=0:1:11 D DRV
	K ^TSX K  W !,*7,"Finished",! Q
DRV	W $J(K,4),?7,$P("For Loop^Do-Quit^String^Pattern^Function^Conversion^Integer^Real^Symbol Tbl^Glb Set^Glb Retrv^Composite","^",K+1)
	K (K,N,T) I $T(@("A"_K))'="" D @("A"_K)
	H 1 S T=$P($H,",",2) D @K S T=$P($H,",",2)-T S:'K T(0)=T/N
	S NN=$S(K<11:N,1:N\10) W ?20,$J(T/NN*1000,8,2),?34,$J(T/NN-T(0)*1000,8,2),! Q
0	F J=1:1:N
	Q
1	F J=1:1:N D A2,A2
	Q
A2	S D="aaa/aaa,aaa,aaa" Q
2	F J=1:1:N S I=$E("abcdefghijklm",3,6)_$P(D,",",2)["cdeg"
	Q
A3	S C="abcd123XX" Q
3	F J=1:1:N I C?1"abc".A3N.E,C?4E1"123"2U
	Q
A4	S A=1,B=0,C=123.456 Q
4	F J=1:1:N S I=$L($S($D(XXX):0,A:$J(C,7,2),1:0))
	Q
A5	S A=10,B=13 Q
5	F J=1:1:N S I=A_B+A_B
	Q
A6	S I=200 Q
6	F J=1:1:N S I=I*I\I+I-I*25\25+25-25
	Q
A7	S R=".222" Q
7	F J=1:1:N S Y=R*R/R+R-R*.125/.125+.125-.125
	Q
A8	S D=""
	F J=81:1:90 S D=D_"a" F I=71:1:90 S @($C(J,I)_"=D")
	K I,J,D Q
8	F J=1:1:N S A=J,A=TG,A=WQ,A=ZZ
	Q
9	F J=1:1:N\500 F Y=1:1:20 S A=$E(123456789,1,Y#10) F X=1:1:25 S ^TSX(Y,X)=A
	Q
10	F J=1:1:N\500 F Y=1:1:20 F X=1:1:25 S C=^TSX(Y,X)
	Q
11	F J=1:1:N\10 D B11
	Q
B11	S A1=1,B1=A1+1,C1=B1_B1,D1=$E(C1,3,99),E1="abcdefg"
	F I=2:1:5,8 S F(I)=$E("123456789",1,I)
	K:$L($D(F(3))) F(3) S L=I*22/3,E=$J(L,6,2)
B11B	S L=$N(F(L)) G:L>-1 B11B
	S A="aaa,",A=A_A_A_"xxxx" I A?3A1P.E S B=$P(A,",",2,3)
	S:B["ppp" B=3 S C="d",@C=123
	K A1,A,C1,C,F Q
