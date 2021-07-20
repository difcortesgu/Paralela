#!/bin/bash

apt-get install -qq gcc-5 g++-5 -y
ln -s /usr/bin/gcc-5 
ln -s /usr/bin/g++-5 

sudo apt-get update
sudo apt-get upgrade

#Install Dependencies
sudo apt-get install -y build-essential 
sudo apt-get install -y cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev

#The following command is needed to process images:
sudo apt-get install -y python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev

#To process videos:
sudo apt-get install -y libavcodec-dev libavformat-dev libswscale-dev libv4l-dev
sudo apt-get install -y libxvidcore-dev libx264-dev

#For GUI:
sudo apt-get install -y libgtk-3-dev

#For optimization:
sudo apt-get install -y libatlas-base-dev gfortran pylint
wget https://github.com/opencv/opencv/archive/4.5.3.zip -O opencv-4.5.3.zip
sudo apt-get install unzip
unzip opencv-4.5.3.zip
%cd opencv-4.5.3
mkdir build
%cd build
cmake -D WITH_TBB=ON -D WITH_OPENMP=ON -D WITH_IPP=ON -D CMAKE_BUILD_TYPE=RELEASE -D BUILD_EXAMPLES=OFF -D WITH_NVCUVID=ON -D WITH_CUDA=OFF -D BUILD_DOCS=OFF -D BUILD_PERF_TESTS=OFF -D BUILD_TESTS=OFF -D WITH_CSTRIPES=ON -D WITH_OPENCL=ON CMAKE_INSTALL_PREFIX=/usr/local/ ..
make -j`nproc`
sudo make install
