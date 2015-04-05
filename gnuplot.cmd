reset
fontsize = 12
set term postscript enhanced eps fontsize
set output "report.eps"
set style fill solid 1.00 border 0
set style histogram errorbars gap 2 lw 1
set style data histogram
set xtics rotate by -45
set ylabel "Micro Seconds"
set yrange [0:*]
set datafile separator ","
set y2tics 50000 nomirror tc lt 2
set y2label 'Bytes' tc lt 2
set y2range [0:*]
set grid ytics  y2tics
plot \
  'report.dat' using 2:3:xtic(1) ti "Encoding Time" linecolor rgb "#FF0000", \
  '' using 4:5 ti "HTTP Request Duration)" lt 1 lc rgb "#0000FF", \
  '' using 6:7 ti "Transfer Size" lt 1 lc rgb "#00FF00" axes x1y2, \
  '' using 8:9 ti "Decode Time" lt 1 lc rgb "#00FFFF" \
  
  
