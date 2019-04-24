#!/bin/bash

check_sudo () {
    if [ $UID != 0 ];then
        echo "Please run with sudo"
        exit 0
    fi
}

rm_image() {
    docker rm -f $(docker ps -a --filter=name=barrelfish -q)
}

docker_run () {
    # check sudo first
    check_sudo
    # run image
    echo "Start barrelfish container"
    docker run -d --name barrelfish -v $(pwd):/root/barrelfish --cap-add SYS_ADMIN barrelfish /bin/bash -c 'sleep 9999999'
    # enter into containers
    docker exec -it $(docker ps --filter=name=barrelfish -aq) /bin/bash
    #rm containers
    echo "quit and remove barrelfish container"
    rm_image
}

docker_run
