#!/bin/bash

make_dir(){
    if [ -e $1 ]; then
        rm -r $1
    fi
    mkdir $1
}

build_openssl(){
    pushd $(pwd)

    cd "3rdparty"
    make_dir "build-openssl"
    cd "build-openssl"

    ../openssl/config --prefix="${ROOT_FOLDER}/openssl" --openssldir="${ROOT_FOLDER}/openssl"
    make -j2
    make install

    popd
}

build_hagoromo(){
    pushd $(pwd)


    work_dir="build-hagoromo"
    make_dir $work_dir
    cd $work_dir

    ${QT_BIN_FOLDER}/qmake ../app/app.pro
    make -j2

    popd
}

deploy_hagoromo(){
    pushd $(pwd)

    build_dir="build-hagoromo"
    work_root_dir="deploy-hagoromo"
    work_dir=${work_root_dir}/hagoromo
    make_dir $work_root_dir
    mkdir -p ${work_dir}/bin
    mkdir -p ${work_dir}/lib

    cp ${build_dir}/Hagoromo ${work_dir}/bin
    cp "openssl/lib/libcrypto.so.1.1" ${work_dir}/lib
    cp "openssl/lib/libssl.so.1.1" ${work_dir}/lib
    cp "LICENSE" ${work_dir}
    cp "README.md" ${work_dir}
    
    cd ${work_root_dir}
    zip -r hagoromo.zip hagoromo/

    popd
}


SCRIPT_FOLDER=$(cd $(dirname $0); pwd)
cd $SCRIPT_FOLDER/..
ROOT_FOLDER=$(pwd)

QT_BIN_FOLDER=$1

if [ -z "${QT_BIN_FOLDER}" ]; then
    echo "usage $(basename $0) QT_BIN_FOLDER"
    echo " QT_BIN_FOLDER   ex: ~/Qt/5.15.2/gcc_64/bin/"
    exit 1
fi

build_openssl
build_hagoromo
deploy_hagoromo
