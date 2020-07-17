#!/bin/bash
if [[ -z `lsmod | grep pteditor` ]]
    then
        echo "Loading pteditor.ko"
        set -x
        sudo insmod pteditor.ko
    else
        echo "Reloading pteditor.ko"
        set -x
        sudo rmmod pteditor
        sudo insmod pteditor.ko
fi
