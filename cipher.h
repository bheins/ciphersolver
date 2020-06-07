#ifndef CIPHER_H
#define CIPHER_H

#include "cipherobjectmenu.h"
#include <QMainWindow>
#include <QHash>
#include <QListWidgetItem>
#include <QRecursiveMutex>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui { class CipherUI; }
QT_END_NAMESPACE

//typedef QVector<QVector<QVector<QString> > > Cipher;

class cipherobj;
class cipher : public QMainWindow
{
    Q_OBJECT

public:
    cipher(QWidget *parent = nullptr);
    ~cipher();

private slots:
    void on_textEdit_textChanged();
    void solver_textChanged(const QString &text);
    void on_cipherwordlineedit_customContextMenuRequested(QPoint pos);
    void on_wordFilter_textEdited(const QString &text);
    void on_wordSelection_itemClicked(QListWidgetItem *item);
    void on_actionSave_triggered(bool);
    void on_actionSave_As_triggered(bool);
    void on_actionOpen_triggered(bool);
    void on_actionClose_triggered(bool);
    void on_actionExit_triggered(bool);

private:
    void load_dictionary();
    void build_interactive_solver();
    void update_cipher();
    void update_solution();
    bool is_untranslated(const QString &symbol);
    void save_cipher(const QString &fileName);
    void remove_unused_symbols();
    cipherobj *find_cipher_by_untranslated_symbol(const QString &symbol);
    cipherobj *find_cipher_by_translated_symbol(const QString& symbol);
    QVector<int> find_all_matching_untranslated_symbols(const int line, const int word, const int symbol_index);
    QString generate_regex_search_string_from_pattern(const QString &text);

    QStringList Dictionary;
    QString CurrentSymbol;
    QString CurrentWord;
    QVector<QVector<QVector<QString> > > TheCipher;
    QMap<int, cipherobj*> TheSolution;
    QScopedPointer<Ui::CipherUI> ui;
    QString SettingsFile;
    QSettings Settings;
};
//Q_DECLARE_METATYPE(Cipher)
//QDataStream & operator<<(QDataStream & dataStream, const Cipher & objectA );
//QDataStream & operator>>(QDataStream & dataStream, Cipher & objectA);
#endif // CIPHER_H
