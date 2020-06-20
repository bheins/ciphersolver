include(../common.pri)
# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE=lib
TARGET=cipher

SOURCES += \
    cipherobject.cpp \
    cipherobjectmenu.cpp \
    ciphertextedit.cpp \
    cipherwordlineedit.cpp \
    cipher.cpp

HEADERS += \
    cipher.h \
    cipherobject.h \
    cipherobjectmenu.h \
    ciphertextedit.h \
    cipherwordlineedit.h

FORMS += \
    cipher.ui
