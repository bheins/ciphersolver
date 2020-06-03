#ifndef CIPHER_H
#define CIPHER_H

#include <QMainWindow>
#include <QMap>
#include <QListWidgetItem>
#include <cipherobjectmenu.h>

QT_BEGIN_NAMESPACE
namespace Ui { class CipherUI; }
QT_END_NAMESPACE


class cipherobj
{
public:
    cipherobj() = default;
    ~cipherobj() = default;

    void add_translation(QString known);
    QString get_translated_symbol() const;
    QString get_translated_symbol_non_const();
    void set_untranslated_symbol(const QString& symbol);
    QString get_untranslated_symbol() const;
    bool is_translated() const;

private:
    bool translatable = true;
    bool translated = false;
    QString untranslated_symbol = "";
    QString translated_symbol = "_";
};

class cipher : public QMainWindow
{
    Q_OBJECT

public:
    cipher(QWidget *parent = nullptr);
    ~cipher();

private slots:
    void on_textEdit_textChanged();
    void solver_textChanged(const QString &text);
    void on_textEdit_customContextMenuRequested(QPoint pos);
    void on_wordFilter_textEdited(const QString &text);
    void on_wordSelection_itemClicked(QListWidgetItem *item);

private:
    void reapply_highlighting_rules();
    void load_dictionary();
    void parse_text();
    void build_interactive_solver();
    bool CipherContainsSymbol(const QString &symbol);
    void UpdateCipher();
    void UpdateSolution();
    void RemoveUnusedSymbolsFromSolution();

    QStringList Dictionary;
    QString CurrentSymbol;
    QString CurrentWord;
    QVector<QVector<QVector<cipherobj> > > TheCipher;
    QMap<QString, QString> TheSolution;
    QScopedPointer<Ui::CipherUI> ui;
    cipherobjectmenu TheMenu;
};
#endif // CIPHER_H
