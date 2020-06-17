include(../common.pri)
TEMPLATE=app
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT += testlib

SOURCES += CipherRegexTest.cpp
LIBS += -lcipher -L../src/$$DESTDIR
