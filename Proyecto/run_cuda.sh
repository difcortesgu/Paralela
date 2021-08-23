#!/bin/bash
nvcc mosaic.cu -o mosaic_cuda `pkg-config --cflags --libs opencv`

for block in 4 8 13 17 21 26; 
do 
    echo bloque $block;
    for thread in 192 384 768 1536 2048;
    do 
        echo hilo $thread;
        for iteracion in {0..9};
        do
            ./mosaic_cuda test.jpg test_output.jpg $block $thread 10 44;
        done;
    done;
done;
