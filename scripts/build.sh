#!/bin/bash -e

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

    if [ "${PLATFORM_TYPE}" == "linux" ]; then
        ../openssl/config --prefix="${ROOT_FOLDER}/openssl" --openssldir="${ROOT_FOLDER}/openssl"
    elif [ "${PLATFORM_TYPE}" == "mac" ]; then
        ../openssl/Configure darwin64-x86_64-cc --prefix="${ROOT_FOLDER}/openssl" --openssldir="${ROOT_FOLDER}/openssl"
    fi

    make -j4
    make install

    popd
}

build_zlib(){
    pushd $(pwd)

    cd "3rdparty"
    make_dir "build-zlib"
    cd "build-zlib"

    if [ "${PLATFORM_TYPE}" == "linux" ]; then
        cmake ../zlib -DCMAKE_INSTALL_PREFIX="../../zlib"
    elif [ "${PLATFORM_TYPE}" == "mac" ]; then
        cmake ../zlib -DCMAKE_INSTALL_PREFIX="../../zlib" -DCMAKE_OSX_ARCHITECTURES="x86_64"
    fi
    cmake --build . --config RELEASE --target install
    cmake --build . --config DEBUG --target install

    cd ../zlib
    git checkout .

    popd
}

build_hagoromo(){
    pushd $(pwd)


    work_dir="build-hagoromo"
    deploy_dir="deploy-hagoromo"
    make_dir $work_dir
    make_dir $deploy_dir
    cd $work_dir

    if [ "${PLATFORM_TYPE}" == "linux" ]; then
        cmake .. -G Ninja -DCMAKE_PREFIX_PATH:PATH="$QTDIR" \
            -DCMAKE_INSTALL_PREFIX:PATH='../deploy-hagoromo/hagoromo' \
            -DHAGOROMO_RELEASE_BUILD=ON \
            -DCMAKE_BUILD_TYPE:STRING=Release
    elif [ "${PLATFORM_TYPE}" == "mac" ]; then
        cmake .. -G Ninja -DCMAKE_PREFIX_PATH:PATH="$QTDIR" \
            -DCMAKE_INSTALL_PREFIX:PATH='../deploy-hagoromo/hagoromo' \
            -DHAGOROMO_RELEASE_BUILD=ON \
            -DCMAKE_BUILD_TYPE:STRING=Release \
            -DCMAKE_OSX_ARCHITECTURES="x86_64"
    fi
    cmake --build . --target update_translations
    cmake --build . --target install

    popd
}

deploy_hagoromo(){
    pushd $(pwd)

    build_dir="build-hagoromo"
    work_root_dir="deploy-hagoromo"
    work_dir=${work_root_dir}/hagoromo

    cp "LICENSE" ${work_dir}
    cp "README.md" ${work_dir}

    if [ "${PLATFORM_TYPE}" == "linux" ]; then
        mkdir -p ${work_dir}/lib

        cp ${SCRIPT_FOLDER}/deploy/Hagoromo.sh ${work_dir}
        if [ $QT_VERSION == 6 ]; then
            deploy_src_list_lib="${SCRIPT_FOLDER}/deploy/linux_lib_6.txt"
            deploy_src_list_plugin="${SCRIPT_FOLDER}/deploy/linux_plugin_6.txt"
            deploy_src_list_qml="${SCRIPT_FOLDER}/deploy/linux_qml_6.txt"
            cp "openssl/lib64/libcrypto.so.3" ${work_dir}/lib
            cp "openssl/lib64/libssl.so.3" ${work_dir}/lib
        else
            deploy_src_list_lib="${SCRIPT_FOLDER}/deploy/linux_lib.txt"
            deploy_src_list_plugin="${SCRIPT_FOLDER}/deploy/linux_plugin.txt"
            deploy_src_list_qml="${SCRIPT_FOLDER}/deploy/linux_qml.txt"
            cp "openssl/lib/libcrypto.so.1.1" ${work_dir}/lib
            cp "openssl/lib/libssl.so.1.1" ${work_dir}/lib
        fi
        cp "zlib/lib/libz.so.1.3.1" ${work_dir}/lib
        cp "zlib/lib/libz.so.1" ${work_dir}/lib

        cat ${deploy_src_list_lib} | xargs -i{} cp -P ${QTDIR}/lib/{} ${work_dir}/lib
        cat ${deploy_src_list_plugin} | xargs -i{} dirname {} | uniq | xargs -i{} mkdir -p ${work_dir}/bin/{}
        cat ${deploy_src_list_plugin} | xargs -i{} cp -P ${QTDIR}/plugins/{} ${work_dir}/bin/{}
        cat ${deploy_src_list_qml} | xargs -i{} dirname {} | uniq | xargs -i{} mkdir -p ${work_dir}/bin/{}
        cat ${deploy_src_list_qml} | xargs -i{} cp -P ${QTDIR}/qml/{} ${work_dir}/bin/{}

    elif [ "${PLATFORM_TYPE}" == "mac" ]; then
        cp -r ${build_dir}/app/Hagoromo.app ${work_dir}/
        ${QTDIR}/bin/macdeployqt ${work_dir}/Hagoromo.app -qmldir=app/qml

        cp -RL "zlib/lib/libz.1.dylib" ${work_dir}/Hagoromo.app/Contents/Frameworks
    fi

    cd ${work_root_dir}
    zip -r hagoromo_${VERSION_NO}_${PLATFORM_TYPE}.zip hagoromo/

    popd
}

update_web(){
    echo ""
    echo "Update web..."
    python3 $SCRIPT_FOLDER/updateweb.py web/layouts/shortcodes/download_link.html $VERSION_NO
    echo "ok"
}

SCRIPT_FOLDER=$(cd $(dirname $0); pwd)
cd $SCRIPT_FOLDER/..
ROOT_FOLDER=$(pwd)

PLATFORM_TYPE=$1
QTDIR=$2

if [[ "$QTDIR" == */6.* ]]; then
QT_VERSION=6
else
QT_VERSION=5
fi

if [ -z "${QTDIR}" ] || [ -z "${PLATFORM_TYPE}" ]; then
    echo "usage $(basename $0) PLATFORM_TYPE QTDIR"
    echo " PLATFORM_TYPE   linux or mac"
    echo " QTDIR           ex: ~/Qt/6.8.1/gcc_64/"
    exit 1
fi

VERSION_NO=$(cat app/main.cpp | grep "app.setApplicationVersion" | grep -oE "[0-9]+.[0-9]+.[0-9]+")

if [ "${PLATFORM_TYPE}" == "mac" ]; then
    PATH=$PATH:$QTDIR/../../Tools/CMake/CMake.app/Contents/bin/
    PATH=$PATH:$QTDIR/../../Tools/Ninja/
fi

build_openssl
build_zlib
build_hagoromo
deploy_hagoromo
# update_web
