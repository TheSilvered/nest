#!/bin/bash
mkdir -p ./unix_libs
cp ../libs/_nest_files/* ./unix_libs

for i in ./unix_libs/*; do
    nest update_nest_file.nest "${i}" so
done
