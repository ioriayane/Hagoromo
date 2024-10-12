win32:{
    bin_dir=$$dirname(QMAKE_QMAKE)
    greaterThan(QT_MAJOR_VERSION, 5) {
        open_ssl_dir=$${bin_dir}/../../../Tools/OpenSSLv3
    }else{
        open_ssl_dir=$${bin_dir}/../../../Tools/OpenSSL
    }
    open_ssl_dir=$$clean_path($$open_ssl_dir)
    SOURCES += $${open_ssl_dir}/src/ms/applink.c

    open_ssl_dir=$${open_ssl_dir}/Win_x64
    LIBS += $${open_ssl_dir}/lib/libssl.lib \
            $${open_ssl_dir}/lib/libcrypto.lib
    INCLUDEPATH += $${open_ssl_dir}/include

    CONFIG(debug,debug|release):install_dir = $$OUT_PWD/debug
    else: install_dir = $$OUT_PWD/release

    depend_files.path = $$install_dir
    greaterThan(QT_MAJOR_VERSION, 5) {
        depend_files.files = \
            $${open_ssl_dir}/bin/libcrypto-3-x64.dll \
            $${open_ssl_dir}/bin/libssl-3-x64.dll
    }else{
        depend_files.files = \
            $${open_ssl_dir}/bin/libcrypto-1_1-x64.dll \
            $${open_ssl_dir}/bin/libssl-1_1-x64.dll
    }
    INSTALLS += depend_files
    # QMAKE_POST_LINK += nmake -f $(MAKEFILE) install

}else{
    open_ssl_dir=$${PWD}
    INCLUDEPATH += $${open_ssl_dir}/include
    mac:{
        LIBS += -L$${open_ssl_dir}/lib -lssl -lcrypto
    }else{
        greaterThan(QT_MAJOR_VERSION, 5) {
            LIBS += -L$${open_ssl_dir}/lib64 -lssl -lcrypto
        }else{
            LIBS += -L$${open_ssl_dir}/lib -lssl -lcrypto
        }
    }
}
