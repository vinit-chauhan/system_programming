#!/bin/bash

echo "Backup script started"

home_dir=/home/`echo $USER`

# temp
home_dir=/media/vinit-chauhan/code/github.com/vinit-chauhan/uwindsor_asp/assignments/assignment-4/home.bkp

exclude_dir=$home_dir/backup

backup_dir=$home_dir/backup/cb
backup_tmp_dir=$backup_dir/_tmp

cd $home_dir

iterate_directory() {
    # iterate over all files in home directory
    for file in `ls $1`; do
        # check if file is a directory
        if [ -d $file ]; then
            temp=`realpath $file`
            
            # check if directory is backup directory
            # if yes then skip
            if [ $temp == $exclude_dir ]; then
                continue
            fi

            mkdir -p $backup_tmp_dir/${temp#"$home_dir"}
            # iterate over all files in directory
            iterate_directory $temp
        
        elif [ -f $file ]; then
            # if file is .c or .txt then backup
            if [[ $file == *.c || $file == *.txt ]]; then
                echo "backing up file: $file"
                cp $file $backup_tmp_dir/$file
            fi
        fi
    done
}


# check if backup directory exists
if [ ! -d $backup_dir ]; then
    mkdir -p $backup_dir
fi

# check if backup tmp directory exists
if [ ! -d $backup_tmp_dir ]; then
    mkdir -p $backup_tmp_dir
fi

iterate_directory $home_dir

echo done
