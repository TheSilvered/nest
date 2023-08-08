#!/bin/bash

src_path=./linux_release/x64;

if ! command -v apt-get &> /dev/null
then
    sudo dnf -y install SDL2 SDL2_ttf
else
    sudo apt-get -y install libsdl2-2.0-0 libsdl2-ttf-2.0-0
fi

sudo mkdir -p /usr/lib/nest/
sudo cp -r ./linux_libs/* /usr/lib/nest
sudo cp ${src_path}/*.cnest /usr/lib/nest
sudo cp ${src_path}/nest /usr/bin
sudo cp ${src_path}/libnest.so /usr/lib
sudo ldconfig
