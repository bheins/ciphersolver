#include "cipherwordlineedit.h"
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>

CipherWordLineEdit::CipherWordLineEdit(QWidget* parent)
    : QLineEdit(parent)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    QFont font("Courier, 16");
    setFont(font);
    setDragEnabled(true);
    setAcceptDrops(true);
    setMinimumWidth(0);
    setMinimumHeight(20);
    setMouseTracking(true);
}

void CipherWordLineEdit::dragEnterEvent(QDragEnterEvent *e)
{
    e->acceptProposedAction();
}

void CipherWordLineEdit::dropEvent(QDropEvent *e)
{
    e->accept();
    QString newText(e->mimeData()->data("application/x-qabstractitemmodeldatalist"));
    if(newText.size() == text().size())
    {
        setText(newText);
    }
    else
    {
        qDebug() << QString("New text (%1) size (%2) does not match current text (%3) size (%4).")
                    .arg(newText)
                    .arg(newText.size())
                    .arg(text())
                    .arg(text().size());
    }
    QLineEdit::dropEvent(e);
}
