#include "cipher.h"
#include "cipher.h"
#include "ui_cipher.h"
#include <QDir>
#include <QTextDocument>
#include <Qt

cipher::cipher(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CipherUI)
{
    ui->setupUi(this);
}

cipher::~cipher()
{
}

void cipher::reapply_highlighting_rules()
{

}

void cipher::load_dictionary()
{
    QDir path(QCoreApplication::applicationDirPath().append("database"));
    QFile database(path.+"\\wordlist.txt");
            if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
}
