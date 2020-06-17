INCLUDEPATH += ../src

CONFIG += c++11
CONFIG(debug, debug|release) {
    DESTDIR = debug
} else {
    DESTDIR = release
}
