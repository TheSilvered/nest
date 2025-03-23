#! /usr/bin/bash

if [[ ! -d /usr/lib/nest || ! -f /usr/bin/nest || ! -f /usr/lib/libnest.so ]]; then
    echo Nest does not seem to be installed, exiting...
    exit 1
fi

if [ $(pgrep -l nest) ]; then
    echo Cannot uninstall Nest while it is running, exiting...
    exit 2
fi

for f in /usr/lib/nest/*; do
    if [ -d "$f" ]; then
        continue
    fi

    if [[ ! $f =~ ^/usr/lib/nest/_?std.*\.c?nest$ ]]; then
        echo File $f is not part of the standard installation, nest cannot be uninstalled. Exiting...
        exit 3
    fi
done

sudo rm -rf /usr/lib/nest
sudo rm -f /usr/bin/nest
sudo rm -f /usr/lib/libnest.so
