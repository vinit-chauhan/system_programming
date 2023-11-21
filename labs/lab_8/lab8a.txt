#!/bin/bash

if [ ! -e "sample.txt" ]; then
    echo "File does not exist, creating one"
    # create the file.
    touch "sample.txt"
fi

# check if you are the owner of the file
if [ ! -O "sample.txt" ]; then
    echo "You are not the owner of the file"
    exit 1
fi

# check if you have write permission
if [ ! -w "sample.txt" ]; then
    echo "File is not writable"
    # add write permission
    chmod u+w "sample.txt"
fi

# append the content of 'ls -1' to 'sample.txt' file
echo "Printing the content of the file"
ls -1 >> "sample.txt"
