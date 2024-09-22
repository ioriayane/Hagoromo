TEMPLATE = subdirs

SUBDIRS += \
    app \
    lib \
    tools/decodecar \
    tools/firehosereader

app.depends = lib

CONFIG(debug, debug|release):{
SUBDIRS += \
    tests
tests.depends = lib
}
