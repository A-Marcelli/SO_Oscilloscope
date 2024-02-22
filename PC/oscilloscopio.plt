#!/usr/local/bin/gnuplot -c --persist
# $ gnuplot -c oscilloscopio.plt NUMERO_ADC MODE DATAFILE_NAME.DAT

if(ARG2+0 == 2)\
    set terminal png size 1800,1000;\
    set output "oscilloscopio.png";\
 else if(ARG2+0 == 1)\
    set terminal wxt


set title "Risultati Oscilloscopio" 
set xlabel "Tempo (ms)"
set ylabel "V (V)"
set autoscale
set grid
set xrange [0:10000]
set yrange [0:3]


if(ARG1+0 == 1)\
    plot ARG3 u 1:2 t 'Canale 1' w lp;\
 else if(ARG1+0 == 2)\
    plot ARG3 u 1:2 t 'Canale 1' w lp,\
         ARG3 u 1:3 t 'Canale 2' w lp;\
 else if(ARG1+0 == 3)\
    plot ARG3 u 1:2 t 'Canale 1' w lp,\
         ARG3 u 1:3 t 'Canale 2' w lp,\
         ARG3 u 1:4 t 'Canale 3' w lp;\
 else if(ARG1+0 == 4)\
    plot ARG3 u 1:2 t 'Canale 1' w lp,\
         ARG3 u 1:3 t 'Canale 2' w lp,\
         ARG3 u 1:4 t 'Canale 3' w lp,\
         ARG3 u 1:5 t 'Canale 4' w lp;\
 else if(ARG1+0 == 5)\
    plot ARG3 u 1:2 t 'Canale 1' w lp,\
         ARG3 u 1:3 t 'Canale 2' w lp,\
         ARG3 u 1:4 t 'Canale 3' w lp,\
         ARG3 u 1:5 t 'Canale 4' w lp,\
         ARG3 u 1:6 t 'Canale 5' w lp;\
 else if(ARG1+0 == 6)\
    plot ARG3 u 1:2 t 'Canale 1' w lp,\
         ARG3 u 1:3 t 'Canale 2' w lp,\
         ARG3 u 1:4 t 'Canale 3' w lp,\
         ARG3 u 1:5 t 'Canale 4' w lp,\
         ARG3 u 1:6 t 'Canale 5' w lp,\
         ARG3 u 1:7 t 'Canale 6' w lp;\
 else if(ARG1+0 == 7)\
    plot ARG3 u 1:2 t 'Canale 1' w lp,\
         ARG3 u 1:3 t 'Canale 2' w lp,\
         ARG3 u 1:4 t 'Canale 3' w lp,\
         ARG3 u 1:5 t 'Canale 4' w lp,\
         ARG3 u 1:6 t 'Canale 5' w lp,\
         ARG3 u 1:7 t 'Canale 6' w lp,\
         ARG3 u 1:8 t 'Canale 7' w lp;\
 else if(ARG1+0 == 8)\
    plot ARG3 u 1:2 t 'Canale 1' w lp,\
         ARG3 u 1:3 t 'Canale 2' w lp,\
         ARG3 u 1:4 t 'Canale 3' w lp,\
         ARG3 u 1:5 t 'Canale 4' w lp,\
         ARG3 u 1:6 t 'Canale 5' w lp,\
         ARG3 u 1:7 t 'Canale 6' w lp,\
         ARG3 u 1:8 t 'Canale 7' w lp,\
         ARG3 u 1:9 t 'Canale 8' w lp;\
