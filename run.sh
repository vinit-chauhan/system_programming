#!/bin/bash

arg=$1

input_file=${arg/.c/}
output_file=outputs/$input_file.out
path=''

# Split the string by /
readarray -d "/" -t dirs <<< "$output_file"

for ((n=0; n < ${#dirs[*]} - 1; n++)) do
    path=${path}/${dirs[n]}
done

mkdir -p ./$path

gcc $input_file.c -o ./${output_file}

./${output_file} "${@:2}"
