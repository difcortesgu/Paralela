#!/bin/bash
cd 1/
g++ test.cpp -g -lpthread -o test `pkg-config --cflags --libs opencv`

hilos[0]=1
hilos[1]=2
hilos[2]=4
hilos[3]=8
hilos[4]=16

imagenes=('hd1' 'fullhd1' '4k1')
for i in {0..2}; 
do for k in {0..4}; 
do for j in {1..10};
do  ./test imagenes/${imagenes[$i]}.jpg ${imagenes[$i]}_output.jpg ${hilos[$k]} 4 >> datosF.txt; 
done ;
done ;
done

imagenes=('hd1' 'fullhd1' '4k1')
for i in {0..2}; 
do for k in {0..4}; 
do for j in {1..10};
do  ./test imagenes/${imagenes[$i]}.jpg ${imagenes[$i]}_output.jpg ${hilos[$k]} 7 >> datosF.txt; 
done ;
done ;
done