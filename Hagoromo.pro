TEMPLATE = subdirs

SUBDIRS += \
    app \
    lib \
    tests \
    tools/decodecar \
    tools/firehosereader

app.depends = lib
tests.depends = lib
