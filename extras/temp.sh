#!/bin/bash

echo $HOME

# export a variable to environment
NAME="vinit"
export NAME

echo $NAME

printenv | grep NAME

echo "directories in $PWD: `ls -l | grep ^d | wc -l`"
echo 'directories in $PWD: `ls -l | grep ^d | wc -l`'


read -p "a: " a
read -p "b: " b

if [ $a -ne 0 ]; then
    echo "a is not zero"
else 
    echo "a is zero"
fi

if [ $b -lt 10 ]; then
    echo "b is less than or equal to 10"
else 
    echo "b is greater than 10"
fi

sum=$((a+b))

echo $sum