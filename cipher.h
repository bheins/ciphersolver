#ifndef CIPHER_H
#define CIPHER_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class CipherUI; }
QT_END_NAMESPACE

class cipher : public QMainWindow
{
    Q_OBJECT

public:
    cipher(QWidget *parent = nullptr);
    ~cipher();

private:
    void reapply_highlighting_rules();
    void load_dictionary();

    QScopedPointer<Ui::CipherUI> ui;
};
#endif // CIPHER_H
