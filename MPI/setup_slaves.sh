#!/bin/bash    
sudo apt-get update
sudo adduser mpiuser --uid 1234
sudo apt-get -y install nfs-common
sudo mount -t nfs master:/home/mpiuser /home/mpiuser/
sudo apt-get -y install openmpi-bin openmpi-common libopenmpi-dev
sudo apt-get -y install g++ libopencv-dev python3-opencv

echo "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABgQDDD7Gzh/xoeAt6hfxGeSiiIO/lUt/Zq6T66+oXvzSrtO9906/uML8pOYfvovvagcBTRF5iEtpsMAPW/CPF1KYWqCd7RODZfcrtIi4YMNRgKhP7mVAPCC4ZaAtfpeYG0cS+AN3gnq2AbTTbDMzz38PvYh5MntvBbr8tNMMttdMMVQVCR+OWnSwlrblTqgL7EglJn8oYLjtQevs4K+ElmFnWEP5Q8rhlKs9m8Luw5z/Hr1KCgfOs9QRpMCc0N98Qex5V9mNZR6gxFyYAdC+5g7g8egEldIEpup9tqX43gmY0dxHZVKowhxSePm29E4z5rvLnFR0sKzxqnMKNkwjMbtMgBONBylxBhOpW5/b6olflK2B4Spt6L3J1U/cMeHzYXle+EbGVy/vp7RlZ5lbPaa3DA/x08Bo1UuyEoIS/1GP1zAETKyX/8/ulO7cIpJJP8mBWT/UybmmhAbcvCm/TvfOrcqcUTQ03/wkcDn2dZxrtnYV4UkFwnX93CRpYCfE+TQE= mpiuser@master" > /home/mpiuser/.ssh/authorized_keys
