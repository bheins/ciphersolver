include(common.pri)
TEMPLATE = subdirs

SUBDIRS += src app test
app.depends = src
test.depends = src

CONFIG += ordered

OTHER_FILES += database/*
