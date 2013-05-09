ref ;;test parameter passing by reference
 s (s,f,x)="" d r(s,f,x,.%)
 w %
 q
r(s,f,x,y)      ;;
 n %
 d rr(s,f,.y,.p)
 q
rr(s,f,x,%) ;;
 n y
 s x="OK!"
 q
