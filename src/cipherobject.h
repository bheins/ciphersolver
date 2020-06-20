#pragma once

#include <QString>
#include <QObject>

class cipherobj : public QObject
{
    Q_OBJECT

public:
    explicit cipherobj(const QString& symbol);
    ~cipherobj() = default;

    void add_translation(const QString &symbol);
    QString get_translated_symbol() const;
    void set_untranslated_symbol(const QString& symbol);
    QString get_untranslated_symbol() const;
    void clear_translation();
    bool is_translated() const;

signals:
    void translation_updated();

private:
    QString untranslated_symbol = "";
    QString translated_symbol = "";
};
