VERSION="0.9.0"

tar -czf "nest-${VERSION}-x64-linux.tar.gz" linux_libs linux_release/x64 linux_install_x64.sh
tar -czf "nest-${VERSION}-x86-linux.tar.gz" linux_libs linux_release/x86 linux_install_x86.sh
