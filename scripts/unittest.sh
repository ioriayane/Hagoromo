#!/bin/bash

make_dir(){
    if [ -e $1 ]; then
        rm -r $1
    fi
    mkdir $1
}


build_hagoromo(){
    pushd $(pwd)


    work_dir="build-hagoromo"
    make_dir $work_dir
    cd $work_dir

    ${QT_BIN_FOLDER}/qmake ../Hagoromo.pro CONFIG+=debug
    make -j4
    if [ $? -ne 0 ]; then
        exit 1
    fi

    popd
}

do_test(){
    $1 --platform offscreen
    if [ $? -ne 0 ]; then
        exit 1
    fi
}

SCRIPT_FOLDER=$(cd $(dirname $0); pwd)
cd $SCRIPT_FOLDER/..
ROOT_FOLDER=$(pwd)

PLATFORM_TYPE=$1
QT_BIN_FOLDER=$2

if [ -z "${QT_BIN_FOLDER}" ] || [ -z "${PLATFORM_TYPE}" ]; then
    echo "usage $(basename $0) PLATFORM_TYPE QT_BIN_FOLDER"
    echo " PLATFORM_TYPE   linux or mac"
    echo " QT_BIN_FOLDER   ex: ~/Qt/5.15.2/gcc_64/bin/"
    exit 1
fi

VERSION_NO=$(cat app/main.cpp | grep "app.setApplicationVersion" | grep -oE "[0-9]+.[0-9]+.[0-9]+")

build_hagoromo

LD_LIBRARY_PATH=$ROOT_FOLDER/openssl/lib
export LD_LIBRARY_PATH

work_dir="./build-hagoromo/tests"
do_test ${work_dir}/atprotocol_test/atprotocol_test
do_test ${work_dir}/chat_test/chat_test
do_test ${work_dir}/hagoromo_test/hagoromo_test
do_test ${work_dir}/hagoromo_test2/hagoromo_test2
do_test ${work_dir}/http_test/http_test
do_test ${work_dir}/search_test/search_test
do_test ${work_dir}/log_test/log_test
do_test ${work_dir}/tools_test/tools_test
