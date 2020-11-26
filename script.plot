set terminal png
set output "test.png"
set title "THIS IS A TITLE"
# key right-center
set xlabel "xlabel"
set ylabel "ylabel"

plot "headsize-400.dat" u 1 w linespoints title ""