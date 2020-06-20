#pragma once
#include <QStringList>

class DatabaseManager
{
public:
    DatabaseManager();

    QStringList filter(const QString &pattern);
    QStringList filter(const QRegExp &pattern);
    QStringList filter(const QRegularExpression &pattern);
    void set_single_substitution(const bool);


private:
    void load_words();
    void load_names();

    QStringList Dictionary;
    bool SingleSubstitution;
    bool SearchFilterLimited;
};
