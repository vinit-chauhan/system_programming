#!/bin/bash

echo "Backup script started"

cb_counter=1
ib_counter=1

home_dir=$HOME

# temp
home_dir=/media/vinit-chauhan/code/github.com/vinit-chauhan/uwindsor_asp/assignments/assignment-4/home.bkp

exclude_dir=$home_dir/backup

backup_dir=$home_dir/backup/cb

backup_tmp_dir=$backup_dir/_tmp

cd $home_dir

cb_iterate_directory() {
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
            cb_iterate_directory $temp

        elif [ -f $file ]; then
            # if file is .c or .txt then backup
            if [[ $file == *.c || $file == *.txt ]]; then
                cp $file $backup_tmp_dir${temp#"$home_dir"}
            fi
        fi
        cd $1
    done
}

ib_iterate_directory() {
    cru_dir=$1
    # iterate over all files in home directory
    for file in $(find $1 -type f -newer "$backup_dir/$name"); do

        temp=${file##"$home_dir/"}

        if [[ $temp == backup/* ]]; then
            continue
        fi

        temp=$backup_tmp_dir${file#$home_dir}

        mkdir -p ${temp%/*}

        if [[ $file == *.c || $file == *.txt ]]; then
            cp $file ${temp%/*}
        fi

        # echo $backup_tmp_dir${file#$home_dir}

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
    # iterate over all files in home directory
    cb_iterate_directory $home_dir

    # generate backup file name
    name=$(generate_file_name cb)

    # create tar file and then remove backup tmp directory
    cd $backup_tmp_dir &&
        tar -cz -f $backup_dir/$name * && cd $home_dir && rm -rf $backup_tmp_dir

    # create backup log entry
    echo $(date) $name was created >>$backup_dir/backup.log

    # increment complete backup counter
    cb_counter=$((cb_counter + 1))
}

incremental_backup() {

    # TODO: get newest file among all files in backup directory
    ib_name=$(generate_file_name ib)
    if [ ! -f $backup_dir/$ib_name ]; then
        name=$(generate_file_name cb)
    else
        name=$ib_name
    fi

    if [ $(find $home_dir -type f -newer "$backup_dir/$name" | wc -l) -gt 0 ]; then

        # iterate over all files in home directory
        ib_iterate_directory $home_dir

        # create tar file and then remove backup tmp directory
        cd $backup_tmp_dir &&
            tar -cz -f $backup_dir/$ib_name * && cd $home_dir && rm -rf $backup_tmp_dir

        # add entry to backup log
        echo $(date) $ib_name 'was created' >>$backup_dir/backup.log

        # increment incremental backup counter
        ib_counter=$((ib_counter + 1))
    else
        echo $(date) No changes-Incremental backup was not created >>$backup_dir/backup.log
    fi

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
    echo "Creating Complete Backup"
    complete_backup
else
    echo "Creating Incremental Backup"
    incremental_backup
fi

echo done
