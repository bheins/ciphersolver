#include "cipher.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    cipher w;
    w.show();
    return a.exec();
}
