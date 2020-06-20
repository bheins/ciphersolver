#pragma once

#include "cipherobjectmenu.h"
#include "databasemanager.h"
#include <QMainWindow>
#include <QHash>
#include <QListWidgetItem>
#include <QRecursiveMutex>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui { class CipherUI; }
QT_END_NAMESPACE

class cipherobj;
class cipher : public QMainWindow
{
    Q_OBJECT

public:
    cipher(QWidget *parent = nullptr);
    ~cipher();

private slots:
    void on_Cipher_textChanged();
    void solver_textChanged(const QString &text);
    void show_custom_word_menu_selection(QPoint pos);
    void on_wordFilter_returnPressed();
    void on_wordSelection_itemClicked(QListWidgetItem *item);
    void on_actionSave_triggered(bool);
    void on_actionSave_As_triggered(bool);
    void actionOther_triggered(bool);
    void on_actionClose_triggered(bool);
    void on_actionExit_triggered(bool);
    void on_PolySymbolic_stateChanged(int);
    void on_SearchboxFiltering_stateChanged(int);
    void on_LoadLast_stateChanged(int);
    void on_ResetCipherButton_clicked();

private:
    void load_dictionary();
    void build_interactive_solver();
    void update_cipher();
    void update_solution();
    bool is_untranslated(const QString &symbol);
    bool save_cipher(const QString &filename);
    void save_cipher_helper(bool allowDiscard);
    void open_cipher(const QString &filename);
    cipherobj *find_cipher_by_untranslated_symbol(const QString &symbol);
    cipherobj *find_cipher_by_translated_symbol(const QString& symbol);
    QVector<int> find_all_matching_untranslated_symbols(const int line, const int word, const int symbol_index);
    QString generate_regex_symbolic_filter(bool limitSearchFilter);
    QString generate_regex_search_string_from_pattern(const QString &text); //word filter
    QString generate_regex_match_restrictions(const QVector<QString> &cipherword);
    void reset();
    void update_recent_files();

    QString CurrentSymbol;
    QString CurrentWord;
    QVector<QVector<QVector<QString> > > TheCipher;
    QMap<int, cipherobj*> TheSolution;
    QScopedPointer<Ui::CipherUI> ui;
    QString SettingsFile;
    QSettings Settings;
    bool CipherChangeNotSaved;
    bool SolverChangeNotSaved;
    bool PolySymbolicSearch;
    bool SearchFilterLimited;
    DatabaseManager DbMgr;
};
