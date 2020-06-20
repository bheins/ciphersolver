#include "databasemanager.h"
#include <QApplication>
#include <QDir>
#include <QTextStream>
#include <QDebug>


DatabaseManager::DatabaseManager()
    : Dictionary()
    , SingleSubstitution(false)
{
    load_words();
    load_names();
}

void DatabaseManager::load_words()
{
    QDir path(QCoreApplication::applicationDirPath().append("\\database"));
    QFile textFile(path.path().append("\\wordlist.txt"));
    QTextStream wordsTextStream(&textFile);

    if(textFile.exists() and textFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while(not wordsTextStream.atEnd())
        {
            QString line = wordsTextStream.readLine();
            if(not line.isNull())
            {
                Dictionary.append(line);
            }
        }
    }
    else
    {
        qDebug() << QString("Error opening %1").arg(textFile.fileName());
    }
}

void DatabaseManager::load_names()
{
    QDir path(QCoreApplication::applicationDirPath().append("\\database"));
    QFile namesFile(path.path().append("\\Given-Names.txt"));
    QTextStream namesTextStream(&namesFile);

    if(namesFile.exists() and namesFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while(not namesTextStream.atEnd())
        {
            QString line = namesTextStream.readLine();
            if(not line.isNull())
            {
                Dictionary.append(line);
            }
        }
    }
    else
    {
        qDebug() << QString("Error opening %1").arg(namesFile.fileName());
    }
}

QStringList DatabaseManager::filter(const QString& pattern)
{
    QStringList results=Dictionary.filter(pattern);
    results.removeDuplicates();
    return results;
}

QStringList DatabaseManager::filter(const QRegExp& pattern)
{
    QStringList results=Dictionary.filter(pattern);
    results.removeDuplicates();
    return results;
}

QStringList DatabaseManager::filter(const QRegularExpression& pattern)
{
    QStringList results=Dictionary.filter(pattern);
    results.removeDuplicates();
    return results;
}

void DatabaseManager::set_single_substitution(const bool singleSubstitution)
{
    qDebug() << QString("Using %1 substitution filter on database results").arg(singleSubstitution ? "single" : "multiple");
    SingleSubstitution = singleSubstitution;
}
