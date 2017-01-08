reset
#set logscale x 2
set grid
set key left
set xlabel 'N'
set ylabel 'time(sec)'
set style fill solid
set title 'Mergesort-concurrent Performance'
#set xrange[0:250000]
#set yrange[0:0.003]
set term png enhanced font 'Verdana,10'
set output 'runtime.png'
set datafile separator ","

plot "output" using 1:2 w lp pt 1 lw 2 title 'Thread_num(1)',\
'' using 1:3 w lp pt 2 lw 2 title 'Thread_num(2)',\
'' using 1:4 w lp pt 3 lw 2 title 'Thread_num(4)',\
'' using 1:5 w lp pt 4 lw 2 title 'Thread_num(8)',\
'' using 1:6 w lp pt 5 lw 2 lc rgbcolor "#808080" title 'Thread_num(16)',\
'' using 1:7 w lp pt 6 lw 2 title 'Thread_num(32)',\
'' using 1:8 w lp pt 7 lw 2 title 'Thread_num(64)',\
