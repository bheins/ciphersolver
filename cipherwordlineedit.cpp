#include "cipherwordlineedit.h"
#include "cipherobject.h"
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>

CipherWordLineEdit::CipherWordLineEdit(const QMap<int, cipherobj*>& word, int lineIndex, int wordIndex, QWidget* parent)
    : QLineEdit(parent)
    , Word(word)
    , Text("")
{
    setObjectName(QString("solver_%1_%2").arg(lineIndex).arg(wordIndex));
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    QFont font("Courier, 16");
    setFont(font);
    setDragEnabled(true);
    setAcceptDrops(true);
    setMinimumWidth(0);
    setMinimumHeight(20);
    setMouseTracking(true);

    QString param;
    for(const auto& cobj : word)
    {
        connect(cobj, &cipherobj::translation_updated, this, &CipherWordLineEdit::on_translation_updated);
    }
    on_translation_updated();
}

QString CipherWordLineEdit::text() const
{
    return Text;
}

void CipherWordLineEdit::setText(const QString & newText)
{
    Text=newText;
    QLineEdit::setText(newText);
}

void CipherWordLineEdit::clear()
{
    Text.clear();
    QLineEdit::clear();
}

void CipherWordLineEdit::dragEnterEvent(QDragEnterEvent *e)
{
    e->acceptProposedAction();
}

void CipherWordLineEdit::dropEvent(QDropEvent *e)
{
    e->accept();
    qDebug() << objectName() << ": " << __FUNCTION__  << ": " << e->mimeData()->text();
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

void CipherWordLineEdit::on_translation_updated()
{
    QString newText;
    QString untranslatedText;
    for(const auto& cobj : Word)
    {
        //qDebug() << objectName() << ":" << cobj->get_untranslated_symbol() << "=" << cobj->get_translated_symbol();
        untranslatedText.append(cobj->get_untranslated_symbol());
        newText.append(cobj->get_translated_symbol());
    }
    Text=newText;
    QLineEdit::setText(Text);
}
