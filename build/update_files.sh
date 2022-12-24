#!/bin/bash
mkdir -p ./unix_libs
cp ../libs/_nest_files/* ./unix_libs

for i in ./unix_libs/*; do
    python3 update_nest_file.py "${i}" so
done
