#include "ciphertextedit.h"
#include <QMouseEvent>

CipherTextEdit::CipherTextEdit(QWidget *parent) : QTextEdit(parent)
{

}

void CipherTextEdit::mousePressEvent(QMouseEvent *mouseEvent)
{
    if (Qt::RightButton == mouseEvent->button()) {
            QTextCursor textCursor = cursorForPosition(mouseEvent->pos());
            textCursor.select(QTextCursor::WordUnderCursor);
            setTextCursor(textCursor);
            QString word = textCursor.selectedText();
            emit ShowContextMenuForWord(word);
        }
        QTextEdit::mousePressEvent(mouseEvent);
}
