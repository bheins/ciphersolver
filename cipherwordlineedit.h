#ifndef CIPHERWORDLINEEDIT_H
#define CIPHERWORDLINEEDIT_H
#include <QLineEdit>

class CipherWordLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit CipherWordLineEdit(QWidget *parent = nullptr);

protected:
    void dropEvent(QDropEvent *e);
    void dragEnterEvent(QDragEnterEvent *);
};

#endif // CIPHERWORDLINEEDIT_H
