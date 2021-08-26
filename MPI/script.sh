#!/bin/bash
mpic++ helloworld.cpp -o helloworld `pkg-config --cflags --libs opencv`

for image in "hd1.jpg" "fullhd1.jpg" "4k1.jpg"
    for nodos in 1 2 4 8;
    do 
        echo hilo $nodos;
        for iteracion in {0..9};
            do
                mpirun -np $nodos -hostfile mpi_hostfile helloworld $image 1; 
        done;
    done;
done;
