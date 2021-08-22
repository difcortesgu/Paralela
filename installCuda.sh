#!/bin/bash

echo CUDA AND NVIDIA INSTALLATION
echo NOTE: THERE IS A QUESTION IN THE INSTALLATION, PLEASE DO NOT FORGET TO ANSWER IT (YOU CAN CHOOSE Y)
wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu1604/x86_64/cuda-repo-ubuntu1604_8.0.61-1_amd64.deb;
dpkg -i cuda-repo-ubuntu1604_8.0.61-1_amd64.deb;
apt-get update -qq;
apt-get install cuda-8.0;
ln -sf /usr/local/cuda-8.0 /usr/local/cuda
import os
os.environ['PATH'] += ':/usr/local/cuda/bin'
os.environ['LD_LIBRARY_PATH'] += ':/usr/local/cuda/lib'

apt-get install gcc-5 g++-5 -y -qq;
ln -s /usr/bin/gcc-5 /usr/local/cuda/bin/gcc;
ln -s /usr/bin/g++-5 /usr/local/cuda/bin/g++;
