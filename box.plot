set terminal png
set output "test.png"
set title "THIS IS A TITLE"
set key right center
set xlabel "xlabel"     
set ylabel "ylabel"
# set logscale y 10
set style fill solid
set boxwidth 0.3
# set xtics rotate by -75
plot "headsize-400.dat" using 1 with boxes title ""