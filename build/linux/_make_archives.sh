VERSION="0.13.1"

mkdir -p linux_archives
tar -czf "linux_archives/nest-${VERSION}-x64-linux.tar.gz" linux_libs linux_release/x64 linux_install_x64.sh
tar -czf "linux_archives/nest-${VERSION}-x86-linux.tar.gz" linux_libs linux_release/x86 linux_install_x86.sh
