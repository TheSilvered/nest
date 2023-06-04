src_path=./linux_release/x86;

sudo mkdir -p /usr/lib/nest/
sudo cp -r ./linux_libs/* /usr/lib/nest
sudo cp ${src_path}/*.cnest /usr/lib/nest
sudo cp ${src_path}/nest /usr/bin
sudo cp ${src_path}/libnest.so /usr/lib
