%ZOS  ;;Stephen G Maher, Netdrive Technology Limited
 ;;Return the operating specific 'uname' information.
 ;;
 Q
INIT ;
 N temp,EXEC,%
 S EXEC="!<uname -a | tr ' ' ':'",temp=""
 X EXEC
 I % D
 . S temp=%(1)
 . Q
 Q temp
 ;
EXAMPLE ;
 S OS=$($ZOS,":")
 I OS="AIX" S ....
 I OS="SunOS" S....
 I OS="HP-UX" S......
 I OS="OSF1" S.....
 Q

