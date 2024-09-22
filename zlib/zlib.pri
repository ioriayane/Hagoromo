zlib_dir=$$PWD

win32:{
    INCLUDEPATH += $${zlib_dir}/include

    CONFIG(debug,debug|release):{
        LIBS += $${zlib_dir}/lib/zlibd.lib
        zlib_depend_files.path = $$OUT_PWD/debug
        zlib_depend_files.files = \
            $${zlib_dir}/bin/zlibd.dll
    }else:{
        LIBS += $${zlib_dir}/lib/zlib.lib
        zlib_depend_files.path = $$OUT_PWD/release
        zlib_depend_files.files = \
            $${zlib_dir}/bin/zlib.dll
    }

    INSTALLS += zlib_depend_files
    # QMAKE_POST_LINK += nmake -f $(MAKEFILE) install
}
unix: {
    INCLUDEPATH += $${zlib_dir}/include
    LIBS += -L$${zlib_dir}/lib -lz
}
