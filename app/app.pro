include(../common.pri)
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TARGET=cipher
CONFIG += file_copies
#CONFIG += embed_manifest_exe
TEMPLATE=app
SOURCES = main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

database.files = $$files(../database/*.txt)
database.path = $$OUT_PWD/$$DESTDIR/database

cipherdll.files=$$files($$OUT_PWD/../src/$$DESTDIR/cipher.dll)
cipherdll.path = $$OUT_PWD/$$DESTDIR

COPIES += database cipherdll
DISTFILES += cipher.manifest \
    copyright.txt
LIBS += -lcipher -L../src/$$DESTDIR

win32 {
    message("win32")
    manifest.files = $$files(cipher.manifest)
    manifest.path = $$OUT_PWD/$$DESTDIR
    COPIES += manifest
    DEPLOY_COMMAND = windeployqt
    DEPLOY_TARGET = $$shell_quote($$shell_path($$DESTDIR\\$${TARGET}.exe))
    libsto_copy.files += ../src/$$DESTDIR/cipher.dll
    QMAKE_MANIFEST=$$PWD/cipher.manifest
#    QMAKE_LFLAGS += /MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"
    QMAKE_POST_LINK = $$quote($${DEPLOY_COMMAND} $${DEPLOY_TARGET})
#    DEPLOY_OPTIONS += "--$$DESTDIR"
#    WINSDK_DIR = C:/Program Files (x86)/Windows Kits/10/
#    WIN_PWD = $$replace(PWD, /, \\)
#    OUT_PWD_WIN = $$replace(OUT_PWD, /, \\)

#    QMAKE_POST_LINK += "$$quote(\"$$WINSDK_DIR\bin\10.0.19041.0\x64\mt.exe\" -manifest \"$$quote($$WIN_PWD\\$$basename(TARGET).manifest)\" -outputresource:$$quote(\"$$OUT_PWD_WIN\\$$DESTDIR\\$${TARGET}.exe\";1))"
}
