include(../common.pri)
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += file_copies
CONFIG += embed_manifest_exe
TEMPLATE=app
SOURCES = main.cpp

DEPLOY_COMMAND = windeployqt
DEPLOY_TARGET = $$shell_quote($$shell_path($$DESTDIR\\$${TARGET}.exe))
DEPLOY_OPTIONS += "--$$DESTDIR"

COPIES += database manifest

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

database.files = $$files(../database/*.txt)
database.path = $$OUT_PWD/$$DESTDIR/database

manifest.files = $$files(cipher.manifest)
manifest.path = $$OUT_PWD/$$DESTDIR

win32 {
    WINSDK_DIR = C:/Program Files (x86)/Windows Kits/10/
    WIN_PWD = $$replace(PWD, /, \\)
    OUT_PWD_WIN = $$replace(OUT_PWD, /, \\)

    QMAKE_POST_LINK = $$quote($${DEPLOY_COMMAND} $${DEPLOY_TARGET})
#    QMAKE_POST_LINK += $$escape_expand(\n\t)
#    QMAKE_POST_LINK += "$$quote(\"$$WINSDK_DIR\bin\10.0.19041.0\x64\mt.exe\" -manifest \"$$quote($$WIN_PWD\\$$basename(TARGET).manifest)\" -outputresource:$$quote(\"$$OUT_PWD_WIN\\$$DESTDIR\\$${TARGET}.exe\";1))"
}
DISTFILES += cipher.manifest
LIBS += -lcipher -L../src/$$DESTDIR
