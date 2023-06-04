src_path="./linux_debug/";

sudo mkdir -p "/usr/lib/nest/font"
sudo cp "./linux_libs/"* /usr/lib/nest
sudo cp "${src_path}/"* /usr/lib/nest
sudo rm /usr/lib/nest/nest
sudo cp "${src_path}/nest" /usr/bin
sudo cp "${src_path}/libnest.so" /usr/lib
sudo cp "../../libs/_nest_files/font/"* /usr/lib/nest/font
