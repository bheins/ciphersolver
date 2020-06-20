include(../common.pri)

TEMPLATE=app
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets gui
QT += testlib
COPIES += database

database.files = $$files(../database/*.txt)
database.path = $$OUT_PWD/$$DESTDIR/database

INCLUDEPATH += ../googletest/googletest
INCLUDEPATH += ../googletest/googletest/include
INCLUDEPATH += ../googletest/googlemock
INCLUDEPATH += ../googletest/googlemock/include

SOURCES += \
    ../googletest/googletest/src/gtest-all.cc \
    ../googletest/googlemock/src/gmock-all.cc \
    CipherWordLineEditTest.cpp

SOURCES += \
    CipherWordLineEditMenuTest.cpp \
    Main.cpp

LIBS += -lcipher -L../src/$$DESTDIR
#LIBS += -lgtest ../googletest/$$DESTDIR/gtest/gtest
#LIBS += -lgmock ../googletest/$$DESTDIR/gmock/gmock

