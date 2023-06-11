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
    make_dir ${work_root_dir}
    mkdir -p ${work_dir}

    cp "LICENSE" ${work_dir}
    cp "README.md" ${work_dir}

    if [ "${PLATFORM_TYPE}" == "linux" ]; then
        mkdir -p ${work_dir}/bin
        mkdir -p ${work_dir}/lib

        cp ${build_dir}/Hagoromo ${work_dir}/bin
        cp ${SCRIPT_FOLDER}/deploy/Hagoromo.sh ${work_dir}
        cp "openssl/lib/libcrypto.so.1.1" ${work_dir}/lib
        cp "openssl/lib/libssl.so.1.1" ${work_dir}/lib

        cat ${SCRIPT_FOLDER}/deploy/linux_lib.txt | xargs -i{} cp -P ${QT_BIN_FOLDER}/../lib/{} ${work_dir}/lib
        cat ${SCRIPT_FOLDER}/deploy/linux_plugin.txt | xargs -i{} dirname {} | uniq | xargs -i{} mkdir -p ${work_dir}/bin/{}
        cat ${SCRIPT_FOLDER}/deploy/linux_plugin.txt | xargs -i{} cp -P ${QT_BIN_FOLDER}/../plugins/{} ${work_dir}/bin/{}
        cat ${SCRIPT_FOLDER}/deploy/linux_qml.txt | xargs -i{} dirname {} | uniq | xargs -i{} mkdir -p ${work_dir}/bin/{}
        cat ${SCRIPT_FOLDER}/deploy/linux_qml.txt | xargs -i{} cp -P ${QT_BIN_FOLDER}/../qml/{} ${work_dir}/bin/{}

    elif [ "${PLATFORM_TYPE}" == "mac" ]; then
        cp -r build-hagoromo/Hagoromo.app deploy-hagoromo/hagoromo/
        ${QT_BIN_FOLDER}/macdeployqt deploy-hagoromo/hagoromo/Hagoromo.app -qmldir=app/qml
    fi

    cd ${work_root_dir}
    zip -r hagoromo_${VERSION_NO}_${PLATFORM_TYPE}.zip hagoromo/

    popd
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

build_openssl
build_hagoromo
deploy_hagoromo
