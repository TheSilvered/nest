#!/bin/bash
src_path="../../unix_release/x64";

if [ "$#" -eq 1 ] && [ "$1" = "x86" ]; then
    src_path="../../unix_release/x86";
fi

sudo mkdir -p "/usr/lib/nest"
sudo cp "./unix_libs/std"* /usr/lib/nest
sudo cp "${src_path}/std"* /usr/lib/nest
sudo cp "${src_path}/nest" /usr/bin
