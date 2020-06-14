#include "cipherobject.h"

cipherobj::cipherobj(const QString& symbol)
{
    setObjectName(QString("Cipherobj_%1").arg(symbol));
    set_untranslated_symbol(symbol);
}

void cipherobj::add_translation(const QString& symbol)
{
    if(symbol not_eq translated_symbol)
    {
        translated_symbol = symbol;
        emit translation_updated();
    }
}

void cipherobj::clear_translation()
{
    translated_symbol="";
    emit translation_updated();
}

QString cipherobj::get_translated_symbol() const
{
    QString symbol = translated_symbol;
    if(symbol.isEmpty())
    {
        symbol = "-";
    }

    return symbol;
}

void cipherobj::set_untranslated_symbol(const QString& symbol)
{
    untranslated_symbol = symbol;
}

QString cipherobj::get_untranslated_symbol() const
{
    return untranslated_symbol;
}

bool cipherobj::is_translated() const
{
    return translated_symbol not_eq "";
}
