#!/bin/bash

echo "Backup script started"

# counter for complete backup and incremental backup
cb_counter=1
ib_counter=1

# flag to check if backup script is stopped
is_stopped=false

# interval between backups ( in seconds )
interval=120

# home directory
home_dir=$HOME

# backup directory
backup_dir=$home_dir/home/backup

# backup directory for complete backup
cb_backup_dir=$backup_dir/cb
cb_backup_tmp_dir=$cb_backup_dir/_tmp

# backup directory for incremental backup
ib_backup_dir=$backup_dir/ib
ib_backup_tmp_dir=$ib_backup_dir/_tmp

# change directory to home directory
cd $home_dir

# function to handle ctrl+c
signal_handler() {
    echo Stopping backup script
    is_stopped=true
}

# register signal handler
trap signal_handler SIGINT

# function to iterate over all files in directory for complete backup
cb_iterate_directory() {
    # iterate over all files in home directory
    for file in $(ls $1); do

        temp=$(realpath $file)

        # check if file is a directory
        if [ -d $file ]; then

            # check if directory is backup directory
            # if yes then skip
            if [ $temp == $backup_dir ]; then
                continue
            fi

            # create directory in backup tmp directory
            mkdir -p $cb_backup_tmp_dir${temp#"$home_dir"}

            cd $file

            # iterate over all files in directory
            cb_iterate_directory $temp

        elif [ -f $file ]; then
            # if file is .c or .txt then backup
            if [[ $file == *.c || $file == *.txt ]]; then
                cp $file $cb_backup_tmp_dir${temp#"$home_dir"}
            fi
        fi
        cd $1
    done
}

# function to iterate over all files in directory for incremental backup
ib_iterate_directory() {
    # iterate over all files in home directory
    for file in $(find $1 -type f -newer "$backup_dir/timestamp"); do

        # strip home directory from file path
        temp=${file##"$home_dir/"}

        # check if file is backup directory
        if [[ $temp == home/backup/* ]]; then
            continue
        fi

        # setup path for backup destination
        temp=$ib_backup_tmp_dir${file#$home_dir}

        # create directory in backup tmp directory
        mkdir -p ${temp%/*}

        # if file is .c or .txt then backup
        if [[ $file == *.c || $file == *.txt ]]; then
            cp $file ${temp%/*}
        fi

    done
}

# function to generate backup file name
generate_file_name() {
    if [[ $1 == cb* ]]; then
        echo "cb0$(printf "%04d" $cb_counter).tar"
    else
        echo "ib1$(printf "%04d" $ib_counter).tar"
    fi
}

# function to create complete backup
complete_backup() {
    # iterate over all files in home directory
    cb_iterate_directory $home_dir

    # generate backup file name
    name=$(generate_file_name cb)

    # create tar file and then remove backup directory from changed files ( to avoid infinite loop )
    cd $cb_backup_tmp_dir && rm -rf $cb_backup_dir/home

    # create tarball of backup tmp directory, and then remove backup tmp directory
    tar -cz -f $cb_backup_dir/$name * && cd $home_dir && rm -rf $cb_backup_tmp_dir

    # create backup log entry
    echo $(date) $name was created >>$backup_dir/backup.log

    # create a timestamp file
    touch $backup_dir/timestamp

    # increment complete backup counter
    cb_counter=$((cb_counter + 1))
}

# function to create incremental backup
incremental_backup() {

    # generate backup file name
    ib_name=$(generate_file_name ib)

    # check if there are any changes in home directory since last backup
    if [ $(find $home_dir -type f -newer "$backup_dir/timestamp" | wc -l) -gt 0 ]; then

        # iterate over all files in home directory
        ib_iterate_directory $home_dir

        # create tar file and then remove backup tmp directory
        cd $ib_backup_tmp_dir && rm -rf $ib_backup_dir/home &&
            tar -cz -f $ib_backup_dir/$ib_name $(ls -A) && cd $home_dir && rm -rf $ib_backup_tmp_dir

        # add entry to backup log
        echo $(date) $ib_name 'was created' >>$backup_dir/backup.log

        # create a timestamp file
        touch $backup_dir/timestamp

        # increment incremental backup counter
        ib_counter=$((ib_counter + 1))
    else # if no changes then add entry to backup log
        echo $(date) No changes-Incremental backup was not created >>$backup_dir/backup.log
    fi

}

check_backup_dir() {
    # check if backup directory exists for incremental backup
    if [ ! -d $ib_backup_dir ]; then
        mkdir -p $ib_backup_dir
    fi

    # check if backup tmp directory exists for incremental backup
    if [ ! -d $ib_backup_tmp_dir ]; then
        mkdir -p $ib_backup_tmp_dir
    fi

    # check if backup directory exists for complete backup
    if [ ! -d $cb_backup_dir ]; then
        mkdir -p $cb_backup_dir
    fi

    # check if backup tmp directory exists for complete backup
    if [ ! -d $cb_backup_tmp_dir ]; then
        mkdir -p $cb_backup_tmp_dir
    fi

    # create backup log file if it does not exist
    if [ ! -f $backup_dir/backup.log ]; then
        touch $backup_dir/backup.log
    fi

}

# infinite loop to run backup script
while (true); do

    check_backup_dir
    complete_backup
    sleep $interval

    if [ $is_stopped == true ]; then
        break
    fi

    check_backup_dir
    incremental_backup
    sleep $interval

    if [ $is_stopped == true ]; then
        break
    fi

    check_backup_dir
    incremental_backup
    sleep $interval

    if [ $is_stopped == true ]; then
        break
    fi
    check_backup_dir
    incremental_backup
    sleep $interval

    if [ $is_stopped == true ]; then
        break
    fi
done
