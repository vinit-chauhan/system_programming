#!/bin/bash

path="$1"
extension="$2"
count=0

# check if we have 2nd argument or not
if [ $# -ne 2 ]; then
    # if we don't, then return count of all files
    count=$(ls "$path" | wc -l)
else
    # if we do, then return count of files with that extension.
    count=$(ls "$path" | grep $2 | wc -l)
fi

# print the output
echo "The count of $2 files in dir $path : $count"