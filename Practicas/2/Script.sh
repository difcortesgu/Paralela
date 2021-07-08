#!/bin/bash
g++ image-effect.cpp -fopenmp -o image-effect `pkg-config --cflags --libs opencv`

hilos[0]=1
hilos[1]=2
hilos[2]=4
hilos[3]=8
hilos[4]=16

imagenes=('hd1' 'fullhd1' '4k1')
for i in {0..2}; 
do for j in {1..7}; 
do for k in {0..4}; 
do  ./image-effect imagenes/${imagenes[$i]}.jpg ${imagenes[$i]}_output.jpg ${hilos[$k]} $j; 
done ;
done ;
done
