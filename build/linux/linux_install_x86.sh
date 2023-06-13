src_path=./linux_release/x86;

if ! command -v apt-get &> /dev/null
then
    sudo dnf -y install SDL2.i686 SDL2_ttf.i686
else
    sudo apt-get -y install libsdl2-2.0-0:i386 libsdl2-ttf-2.0-0:i386
fi

sudo mkdir -p /usr/lib/nest/
sudo cp -r ./linux_libs/* /usr/lib/nest
sudo cp ${src_path}/*.cnest /usr/lib/nest
sudo cp ${src_path}/nest /usr/bin
sudo cp ${src_path}/libnest.so /usr/lib
sudo ldconfig
