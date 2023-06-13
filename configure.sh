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
    sudo dnf -y install pkg-config
else
    sudo apt-get -y update
    sudo apt-get -y upgrade
    sudo apt-get -y install build-essential gcc-multilib g++-multilib
    sudo apt-get -y install pkg-config
fi

if [ $x64 = true ]; then
    if ! command -v apt-get &> /dev/null
    then
        sudo dnf -y install SDL2-devel SDL2_ttf-devel
    else
        sudo dpkg --add-architecture amd64
        sudo apt-get -y install libsdl2-dev libsdl2-ttf-dev
    fi
fi

if [ $x86 = true ]; then
    if ! command -v apt-get &> /dev/null
    then
        sudo dnf -y install SDL2-devel.i686 SDL2_ttf-devel.i686
    else
        sudo dpkg --add-architecture i386
        sudo apt-get -y install libsdl2-dev:i386 libsdl2-ttf-dev:i386
    fi
fi
