#pragma once

#include <QTextEdit>

class CipherTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit CipherTextEdit(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *mouseEvent);

signals:
    void ShowContextMenuForWord(const QString& word);
};
