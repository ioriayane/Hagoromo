#!/bin/bash -e

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

    if [ "${PLATFORM_TYPE}" == "linux" ]; then
        cmake .. -G Ninja -DCMAKE_PREFIX_PATH:PATH="$QTDIR" \
            -DCMAKE_INSTALL_PREFIX:PATH='../deploy-hagoromo/hagoromo' \
            -DHAGOROMO_UNIT_TEST_BUILD=ON \
            -DCMAKE_BUILD_TYPE:STRING=Debug
    elif [ "${PLATFORM_TYPE}" == "mac" ]; then
        cmake .. -G Ninja -DCMAKE_PREFIX_PATH:PATH="$QTDIR" \
            -DCMAKE_INSTALL_PREFIX:PATH='../deploy-hagoromo/hagoromo' \
            -DHAGOROMO_UNIT_TEST_BUILD=ON \
            -DCMAKE_BUILD_TYPE:STRING=Debug \
            -DCMAKE_OSX_ARCHITECTURES="x86_64"
    fi
    cmake --build . --target tests/all

    popd
}

SCRIPT_FOLDER=$(cd $(dirname $0); pwd)
cd $SCRIPT_FOLDER/..
ROOT_FOLDER=$(pwd)

PLATFORM_TYPE=$1
QTDIR=$2

if [ -z "${QTDIR}" ] || [ -z "${PLATFORM_TYPE}" ]; then
    echo "usage $(basename $0) PLATFORM_TYPE QTDIR"
    echo " PLATFORM_TYPE   linux or mac"
    echo " QTDIR           ex: ~/Qt/5.15.2/gcc_64/"
    exit 1
fi

VERSION_NO=$(cat app/main.cpp | grep "app.setApplicationVersion" | grep -oE "[0-9]+.[0-9]+.[0-9]+")

if [ "${PLATFORM_TYPE}" == "mac" ]; then
    PATH=$PATH:$QTDIR/../../Tools/CMake/CMake.app/Contents/bin/
    PATH=$PATH:$QTDIR/../../Tools/Ninja/
fi

build_hagoromo

if [ "${PLATFORM_TYPE}" == "mac" ]; then
    mkdir -p build-hagoromo/tests/Frameworks
    cp zlib/lib/libz.1.* build-hagoromo/tests/Frameworks
else
    LD_LIBRARY_PATH=$ROOT_FOLDER/openssl/lib64:$ROOT_FOLDER/zlib/lib
    export LD_LIBRARY_PATH
fi

work_dir="./build-hagoromo"
cd $work_dir
ctest --test-dir tests -C Debug -j 4
