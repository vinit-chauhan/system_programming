#!/bin/bash

str=""

# check if there are no arguments
if [ $# -eq 0 ]; then
    echo "No arguments provided"
    exit 1
# check if there are more than 4 arguments
elif [ $# -gt 4 ]; then
    echo "More than 4 arguments provided"
    exit 1
fi

# Get the total number of arguments
total_args=$#

# Start the loop from the last argument
until [ $total_args -eq 0 ]; do
    # Get the current argument index
    current_arg=$((total_args))

    # check if the file has .txt extension
    if [[ ! -f ${!current_arg} ]]; then
        echo "File ${!current_arg} does not exist"
        exit 1
    elif [[ ${!current_arg} != *.txt ]]; then
        echo "File ${!current_arg} does not have .txt extension"
        exit 1
    fi

    # print file to the string variable
    str=$str$(cat ${!current_arg})

    # Decrement the total_args counter
    total_args=$((total_args - 1))
done

# print string to the file
echo "$str" > result.txt
