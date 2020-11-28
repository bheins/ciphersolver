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
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setTextMargins(1,1,1,1);
    setMinimumSize(5,0);
    setDragEnabled(true);
    setAcceptDrops(true);
    setMouseTracking(true);
    setFont(QFont("Courier", 16, QFont::Normal));

    QString param;
    for(const auto& cobj : word)
    {
        connect(cobj, &cipherobj::translation_updated, this, &CipherWordLineEdit::translation_updated);
    }
    translation_updated();
}

CipherWordLineEdit::~CipherWordLineEdit()
{
}

QString CipherWordLineEdit::text() const
{
    return Text;
}

void CipherWordLineEdit::setText(const QString & newText)
{
    qDebug() << reinterpret_cast<uintptr_t>(this);
    Text=newText;
    QLineEdit::setText(newText);
}

void CipherWordLineEdit::clear()
{
    qDebug() << __PRETTY_FUNCTION__;
    for(const auto& cobj : Word)
    {
        cobj->clear_translation();
    }
//    Text.clear();
//    QLineEdit::clear();
}

void CipherWordLineEdit::dragEnterEvent(QDragEnterEvent *e)
{
    e->acceptProposedAction();
}

void CipherWordLineEdit::dropEvent(QDropEvent *e)
{
    if(e->source() not_eq this)
    {
        e->accept();
        qDebug() << objectName() << ": " << __FUNCTION__  << ": " << sender()->objectName();
        QString newText(e->mimeData()->data("application/x-qabstractitemmodeldatalist"));
        if(newText.size() == text().size())
        {
            setText(newText);
        }
        QLineEdit::dropEvent(e);
    }
    else
    {
        e->ignore();
    }
}

void CipherWordLineEdit::translation_updated()
{
    QString newText;
    QString untranslatedText;
    for(const auto& cobj : Word)
    {
        untranslatedText.append(cobj->get_untranslated_symbol());
        newText.append(cobj->get_translated_symbol());
    }
    Text=newText;
    QLineEdit::setText(Text);
    QFontMetrics fm(this->font());
    setFixedSize(fm.horizontalAdvance(Text)+textMargins().left()+textMargins().right()+4,
                 fm.height()+textMargins().top()+textMargins().bottom()+4);
}
