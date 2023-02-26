src_path="./linux_debug/";

sudo mkdir -p "/usr/lib/nest"
sudo cp "./linux_libs/std"* /usr/lib/nest
sudo cp "${src_path}/std"* /usr/lib/nest
sudo cp "${src_path}/nest" /usr/bin
sudo cp "${src_path}/libnest.so" /usr/lib
