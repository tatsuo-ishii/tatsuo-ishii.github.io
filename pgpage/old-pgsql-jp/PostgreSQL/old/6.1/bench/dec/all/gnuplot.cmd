set yrange [0:3]
set ylabel "sec"
set data style linespoints
set terminal pbm color
plot  "cc.data" title "DEC cc (-migrate -O5 -inline speed -unroll 1)" , "gcc2.data" title "gcc-2.7.2.3 (-O3 -funroll-loops)" , "gcc3.data" title "gcc without disk array(-O3 -funroll-loops)" , "gcc1.data" title "gcc-no optimize" 
