#!/bin/bash

sudo mknod /dev/core_monitor c 241 0
sudo insmod core_monitor_lkm.ko
sudo chmod 666 /dev/core_monitor
lsmod | grep core
make
gcc -O2 core_manager.c -lmicrohttpd -o core_monitor
sudo ./core_monitor
