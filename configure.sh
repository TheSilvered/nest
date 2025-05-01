#! usr/bin/bash
x64=true
x86=true

if [ "$#" -eq 1 ] && [ "$1" = "x64" ]; then
    x86=false
fi

if [ "$#" -eq 1 ] && [ "$1" = "x86" ]; then
    x64=false
fi

if ! command -v apt-get &> /dev/null
then
    sudo dnf -y check-update
    sudo dnf -y update
    sudo dnf -y install gcc g++ glibc-devel.i686 libstdc++-devel.i686
    sudo dnf -y groupinstall "Development Tools"
else
    sudo apt-get -y update
    sudo apt-get -y upgrade
    sudo apt-get -y install build-essential gcc-multilib g++-multilib
fi
