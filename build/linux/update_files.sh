mkdir -p ./linux_libs
cp -r ../../libs/_nest_files/* ./linux_libs

if [ "$#" -eq 1 ] && [ "$1" = "py" ]; then
    for i in ./linux_libs/*.nest; do
        python3 ../update_nest_file.py "${i}"
    done
else
    for i in ./linux_libs/*.nest; do
        nest ../update_nest_file.nest "${i}"
    done
fi
