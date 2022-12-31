#!/bin/bash
mkdir -p ./linux_libs
cp ../../libs/_nest_files/* ./linux_libs

if [ "$#" -eq 1 ] && [ "$1" = "py" ]; then
    for i in ./linux_libs/*; do
        python3 ../update_nest_file.py "${i}" so
    done
else
    for i in ./linux_libs/*; do
        nest ../update_nest_file.nest "${i}" so
    done
fi
