#!/bin/bash

echo "Backup script started"

cb_counter=0
ib_counter=0

home_dir=$HOME

# temp
home_dir=/media/vinit-chauhan/code/github.com/vinit-chauhan/uwindsor_asp/assignments/assignment-4/home.bkp

exclude_dir=$home_dir/backup

backup_dir=$home_dir/backup/cb

backup_tmp_dir=$backup_dir/_tmp

cd $home_dir

iterate_directory() {
    # iterate over all files in home directory
    for file in $(ls $1); do

        temp=$(realpath $file)

        # check if file is a directory
        if [ -d $file ]; then

            # check if directory is backup directory
            # if yes then skip
            if [ $temp == $exclude_dir ]; then
                continue
            fi

            # create directory in backup tmp directory
            mkdir -p $backup_tmp_dir${temp#"$home_dir"}

            cd $file

            # iterate over all files in directory
            iterate_directory $temp

        elif [ -f $file ]; then
            # if file is .c or .txt then backup
            if [[ $file == *.c || $file == *.txt ]]; then
                cp $file $backup_tmp_dir${temp#"$home_dir"}
            fi
        fi
        cd $1
    done
}

generate_file_name() {
    if [[ $1 == cb* ]]; then
        echo "cb0$(printf "%04d" $cb_counter).tar"
    else
        echo "ib1$(printf "%04d" $ib_counter).tar"
    fi
}

complete_backup() {

    iterate_directory $home_dir

    # generate backup file name
    name=$(generate_file_name cb)

    # create tar file and then remove backup tmp directory
    tar -czf $backup_dir/$name -C $backup_tmp_dir * && rm -rf $backup_tmp_dir

    # create backup log entry
    echo $(date) $name was created >>$backup_dir/backup.log

    # increment complete backup counter
    cb_counter=$((cb_counter + 1))
}

incremental_backup() {

    # find files that changed after the last backup
    files=$(find $home_dir -type f -newer $backup_dir/backup.log)

    # iterate over all files that changed
    for file in $files; do

        temp=$(realpath $file)

        # check if file is a directory
        if [ -d $file ]; then

            # check if directory is backup directory
            # if yes then skip
            if [ $temp == $exclude_dir ]; then
                continue
            fi

            # create directory in backup tmp directory
            mkdir -p $backup_tmp_dir${temp#"$home_dir"}

            cd $file

            # iterate over all files in directory
            iterate_directory $temp

        elif [ -f $file ]; then
            # if file is .c or .txt then backup
            if [[ $file == *.c || $file == *.txt ]]; then
                cp $file $backup_tmp_dir${temp#"$home_dir"}
            fi
        fi
        cd $1
    done

    # create tar file
    tar -czf $backup_dir/incremental_backup.tar.gz -C $backup_tmp_dir *

    # remove backup tmp directory
    rm -rf $backup_tmp_dir
}

# check if backup directory exists
if [ ! -d $backup_dir ]; then
    mkdir -p $backup_dir
fi

# check if backup tmp directory exists
if [ ! -d $backup_tmp_dir ]; then
    mkdir -p $backup_tmp_dir
fi

# create backup log
if [ ! -f $backup_dir/backup.log ]; then
    touch $backup_dir/backup.log
fi

# check if backup is required
if [ ! -f $backup_dir/$(generate_file_name cb) ]; then

    complete_backup

else

    name=$(generate_file_name ib)

    if [ $(find $home_dir -type f -newer "$backup_dir/$name" | wc -l) -gt 0 ]; then
        incremental_backup
        echo $(date) $name 'was created' >>$backup_dir/backup.log
    else
        echo $(date) No changes-Incremental backup was not created >>$backup_dir/backup.log
    fi
fi

echo done

# # filees that changed after backup
# find $backup_tmp_dir -type f -newer $backup_dir/backup.log -exec echo {} \; > $backup_dir/backup.log

# touch -t 201112220000 start
# touch -t 201112240000 stop
# find . -newer start \! -newer stop
