#!/bin/bash
sudo apt-get update
sudo adduser mpiuser --uid 1234
sudo apt-get -y install g++ libopencv-dev python3-opencv
sudo apt-get -y install openmpi-bin openmpi-common libopenmpi-dev
sudo apt-get -y install nfs-kernel-server
sudo apt-get -y install nfs-common

sudo echo "/home/mpiuser *(rw,sync,no_subtree_check)" > /etc/exports
sudo service nfs-kernel-server restart
sudo exportfs -a

mpic++ helloworld.cpp -o helloworld `pkg-config --cflags --libs opencv` && mpirun -np 4 -hostfile mpi-hostfile helloworld