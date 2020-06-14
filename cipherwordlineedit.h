#pragma once

#include <QLineEdit>
#include <QString>

class cipherobj;
class CipherWordLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    CipherWordLineEdit(const QMap<int, cipherobj *> &, const int lineIndex, const int word, QWidget *parent = nullptr);
    ~CipherWordLineEdit();

    QString text() const;
    void setText(const QString &newText);
    void clear();

protected:
    void dropEvent(QDropEvent *e) override;
    void dragEnterEvent(QDragEnterEvent *) override;

private slots:
    void translation_updated();

private:
    QMap<int, cipherobj*> Word;
    QString Text;
};
