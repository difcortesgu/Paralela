#!/bin/bash
g++ -std=c++17 mosaic.cpp -fopenmp -o mosaic_omp `pkg-config --cflags --libs opencv4`

for thread in 1 2 4 8 16;
do
    echo hilo $thread;
    for iteracion in {0..9};
    do
        ./mosaic_omp test.jpg test_output.jpg $thread 10 44;
    done;
done;