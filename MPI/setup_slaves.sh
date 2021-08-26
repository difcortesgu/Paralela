#!/bin/bash    
sudo apt-get update
sudo adduser mpiuser --uid 1111
sudo apt-get -y install nfs-common
sudo mount -t nfs 10.128.0.12:/home/mpiuser /home/mpiuser/
sudo apt-get -y install openmpi-bin openmpi-common libopenmpi-dev
sudo apt-get -y install g++ libopencv-dev python3-opencv

su mpiuser && cd ~ && echo "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQC+BQ2YD5oekeMvkVaZvlZ4YJShTeartTf+tS5YQ9iDMbg2oHZrMR5DjV6zlCJsvjrrBrqjSToYgC8eCh2cLxI+mqe7o9jeq3r91T4qU9NV3jkNlRbx6sv86z3q89OUXe/FLuwGmntciN3Mxw9npdzzzACZ07UZthpuWHtalf3YT47GhaUs2AG5IoeB0VtBMQJTRg03rU4tym2qivr7HrbIEhth0mduTJbGPjrFygCf0TLgH4PoB/tWn//b0gB6EzdmKXkBIWQ77almLxEhgU5KcJ7l3f7OojuPUssmFfA8gObi/9idN1Dm/9yjmPA3LmT1ZJN7q4PJGU+wdWSn+OIP mpiuser@vms-paralela-f5ch" > /home/mpiuser/.ssh/authorized_keys
