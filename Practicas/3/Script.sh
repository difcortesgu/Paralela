#!/bin/bash
# nvcc image-effect.cu -o image-effect `pkg-config --cflags --libs opencv`

imagenes=('hd1' 'fullhd1' '4k1')
for i in {0..2}; 
do 
    for block in 10 20 30 40 50 60; 
    do 
        echo bloque $block;
        for thread in 64 128 256 512 1024;
        do 
            echo hilo $thread;
            for iteracion in {0..9};
            do
                echo "hellooo";
                # do  ./image-effect imagenes/${imagenes[$i]}.jpg ${imagenes[$i]}_output.jpg $block $thread; 
            done ;
        done ;
    done ;
done
