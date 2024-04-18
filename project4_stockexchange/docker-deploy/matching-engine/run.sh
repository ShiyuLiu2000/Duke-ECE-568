#!/bin/bash
make clean
make
echo 'Running our matching engine server...'
./server
while true
do
    sleep 1
done