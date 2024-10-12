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
        PATH=$PATH:$QT_BIN_FOLDER/../../../Tools/CMake/CMake.app/Contents/bin/
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
    make_dir $work_dir
    cd $work_dir

    ${QT_BIN_FOLDER}/qmake ../Hagoromo.pro CONFIG+=HAGOROMO_RELEASE_BUILD
    make -j4

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
        mkdir -p ${work_dir}/bin/translations
        mkdir -p ${work_dir}/lib

        cp ${build_dir}/app/Hagoromo ${work_dir}/bin
        cp ${SCRIPT_FOLDER}/deploy/Hagoromo.sh ${work_dir}
        if [ $QT_VERSION == 6 ]; then
            cp "openssl/lib64/libcrypto.so.3" ${work_dir}/lib
            cp "openssl/lib64/libssl.so.3" ${work_dir}/lib
        else
            cp "openssl/lib/libcrypto.so.1.1" ${work_dir}/lib
            cp "openssl/lib/libssl.so.1.1" ${work_dir}/lib
        fi
        cp "zlib/lib/libz.so.1.3.1" ${work_dir}/lib
        cp "zlib/lib/libz.so.1" ${work_dir}/lib
        cp "app/i18n/app_ja.qm" ${work_dir}/bin/translations
        cp "lib/i18n/lib_ja.qm" ${work_dir}/bin/translations
        cp ${QT_BIN_FOLDER}/../translations/qt_ja.qm ${work_dir}/bin/translations

        cat ${SCRIPT_FOLDER}/deploy/linux_lib.txt | xargs -i{} cp -P ${QT_BIN_FOLDER}/../lib/{} ${work_dir}/lib
        cat ${SCRIPT_FOLDER}/deploy/linux_plugin.txt | xargs -i{} dirname {} | uniq | xargs -i{} mkdir -p ${work_dir}/bin/{}
        cat ${SCRIPT_FOLDER}/deploy/linux_plugin.txt | xargs -i{} cp -P ${QT_BIN_FOLDER}/../plugins/{} ${work_dir}/bin/{}
        cat ${SCRIPT_FOLDER}/deploy/linux_qml.txt | xargs -i{} dirname {} | uniq | xargs -i{} mkdir -p ${work_dir}/bin/{}
        cat ${SCRIPT_FOLDER}/deploy/linux_qml.txt | xargs -i{} cp -P ${QT_BIN_FOLDER}/../qml/{} ${work_dir}/bin/{}

    elif [ "${PLATFORM_TYPE}" == "mac" ]; then
        cp -r ${build_dir}/app/Hagoromo.app ${work_dir}/
        ${QT_BIN_FOLDER}/macdeployqt ${work_dir}/Hagoromo.app -qmldir=app/qml

        mkdir -p ${work_dir}/Hagoromo.app/Contents/MacOS/translations
        cp "app/i18n/app_ja.qm" ${work_dir}/Hagoromo.app/Contents/MacOS/translations
        cp "lib/i18n/lib_ja.qm" ${work_dir}/Hagoromo.app/Contents/MacOS/translations
        cp ${QT_BIN_FOLDER}/../translations/qt_ja.qm ${work_dir}/Hagoromo.app/Contents/MacOS/translations
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
QT_BIN_FOLDER=$2

if [[ "$QT_BIN_FOLDER" == */6.* ]]; then
QT_VERSION=6
else
QT_VERSION=5
fi

if [ -z "${QT_BIN_FOLDER}" ] || [ -z "${PLATFORM_TYPE}" ]; then
    echo "usage $(basename $0) PLATFORM_TYPE QT_BIN_FOLDER"
    echo " PLATFORM_TYPE   linux or mac"
    echo " QT_BIN_FOLDER   ex: ~/Qt/5.15.2/gcc_64/bin/"
    exit 1
fi

if [ "${PLATFORM_TYPE}" = "mac" ]; then
    if [ "$(uname -m)" != "x86_64" ]; then
        echo "============ Warning ================"
        echo "  Requires startup in x86_64 mode"
        echo "====================================="
        echo "usage arch -x86_64 $(basename $0) PLATFORM_TYPE QT_BIN_FOLDER"
        echo " PLATFORM_TYPE   linux or mac"
        echo " QT_BIN_FOLDER   ex: ~/Qt/5.15.2/gcc_64/bin/"
        exit 1
    fi
fi

VERSION_NO=$(cat app/main.cpp | grep "app.setApplicationVersion" | grep -oE "[0-9]+.[0-9]+.[0-9]+")

build_openssl
build_zlib
build_hagoromo
deploy_hagoromo
# update_web
