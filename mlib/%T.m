%T	; A.Trocha; get current time  01/28/1999 01:39/GMT+1
	; $Source: /cvsroot-fuse/gump/FreeM/mlib/%T.m,v $
	; $Revision: 1.3 $ $Date: 2000/02/18 15:13:42 $
	;    %TIM   <hh>:<mm>        (<hh>: 0-23)
	;    %TIM1  <hh>:<mm> am|pm  (<hh>: 1-12)
	;    %TIM2  <hh>:<mm>:<ss>   (<hh>: 0-23)
	;
	N %TIM,%TIM1,%TIM2
	D INT
	W %TIM1
	Q
	
INT	N time,hh,hh2,mm,ss
	S time=$P($H,",",2)
	S hh=time\3600
	S mm=time-(hh*3600)\60
	S ss=time-(hh*3600)-(mm*60)
	S %TIM=hh_":"_(mm\10)_(mm#10) ; like MSM
	S hh2=(hh-1)#12+1
	S %TIM1=$J(hh2,2)_":"_(mm\10)_(mm#10)_" "_$S(hh\13:"PM",1:"AM") ; like MSM
	S %TIM2=%TIM_":"_(ss\10)_(ss#10)
	Q
