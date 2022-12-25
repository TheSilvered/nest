#!/bin/bash
mkdir -p ./unix_libs
cp ../../libs/_nest_files/* ./unix_libs

if [ "$#" -eq 1 ] && [ "$1" = "py" ]; then
    for i in ./unix_libs/*; do
        python3 ../update_nest_file.py "${i}" so
    done
else
    for i in ./unix_libs/*; do
        nest ../update_nest_file.nest "${i}" sudo
    done
fi