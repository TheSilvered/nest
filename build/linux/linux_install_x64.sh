src_path="./linux_release/x64";

sudo mkdir -p "/usr/lib/nest"
sudo cp "./linux_libs/"* /usr/lib/nest
sudo cp "${src_path}/"* /usr/lib/nest
sudo cp "${src_path}/nest" /usr/bin
sudo cp "${src_path}/libnest.so" /usr/lib
