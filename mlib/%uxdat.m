%uxdat(h,format)        ; FreeM Date/Time formatting  - version 0.5.0.1
 ; Jon Diamond - 1999-03-21
 ;
 n time
 ;
 S h=$G(h) i h="" s h=+$H
 s time=$P(h,",",2) i time="" s time=$p($h,",",2)
 q $$date(h,$g(format))_"  "_$$time(time,$g(format))
 ;
date(h,format)   
 s format=$g(format)     
 n day,h,i,ly,mon,r,year,mm
 S h=$G(h) i h="" s h=+$H
 s time=$P(h,",",2) i time="" s time=$p($h,",",2)
         
 s h=h>21914+h ; 1900 IS NOT A LEAP YEAR
 S ly=h\1461,r=h#1461,year=ly*4+1841+(r\365),day=r#365,mon=1
 I r=1460,ly'=14 S day=365,year=year-1
 F i=31,(r>1154)&(ly'=14)+28,31,30,31,30,31,31,30,31,30 Q:i'<day  S mon=mon+1,day=day-i
 I day=0 S year=year-1,mon=12,day=31
 ; 
 S mm=$P("JAN FEB MAR APR MAY JUN JUL AUG SEP OCT NOV DEC"," ",mon)
 I day<10 S day=0_day
 I mon<10 S mon=0_mon
 i +format=1 q year_"-"_mon_"-"_day
 q day_"-"_mm_"-"_year
 ; 
time(time,format)
 S time=$g(time) i time="" s time=$p($h,",",2)
 n %TIM,%TIM1,%TIM2,hour,hour2,min,sec
 S hour=time\3600
 S min=time-(hour*3600)\60
 S sec=time-(hour*3600)-(min*60)
 S %TIM=hour_":"_(min\10)_(min#10) ; like MSM
 S hour2=(hour-1)#12+1
 S %TIM1=$J(hour2,2)_":"_(min\10)_(min#10)_" "_$S(hour\13:"PM",1:"AM") ; like MSM
 S %TIM2=%TIM_":"_(sec\10)_(sec#10)       
 ;       
 i +format=1 q %TIM2
 q %TIM1

