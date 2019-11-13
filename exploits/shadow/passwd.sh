#!/bin/bash

if [ -n "$1" ]
then
    echo "while: taskset -c $1 passwd -S $USER > /dev/null;"
    while true;
        do taskset -c $1 passwd -S $USER > /dev/null;
    done;
else
    echo 'call with: ./passwd.sh "logical core"'
fi
