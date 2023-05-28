x64=true
x86=true

if [ "$#" -eq 1 ] && [ "$1" = "x64" ]; then
    x86=false
fi

if [ "$#" -eq 1 ] && [ "$1" = "x86" ]; then
    x64=false
fi

# Upgrade packages
sudo apt-get --y update
sudo apt-get --y upgrade

# Install gcc
sudo apt-get --y install build-essential gcc-multilib g++-multilib
# Install package config
sudo apt-get --y install pkg-config

if [ $x64 = true ]; then
    # Install SDL2 x64
    sudo dpkg --add-architecture amd64
    # sudo apt-get --y update
    # sudo apt-get --y upgrade
    sudo apt-get --y install libsdl2-dev libsdl2-ttf-dev
fi

if [ $x86 = true ]; then
    # Instal SDL2 x86
    sudo dpkg --add-architecture i386
    # sudo apt-get --y update
    # sudo apt-get --y upgrade
    sudo apt-get --y install libsdl2-dev:i386 libsdl2-ttf-dev:i386
fi
