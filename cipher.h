#ifndef CIPHER_H
#define CIPHER_H

#include <QMainWindow>
#include <QMap>
#include <cipherobjectmenu.h>

QT_BEGIN_NAMESPACE
namespace Ui { class CipherUI; }
QT_END_NAMESPACE

class cipher : public QMainWindow
{
    Q_OBJECT

    class cipherobj
    {
    public:
        cipherobj() = default;
        ~cipherobj() = default;

        bool translatable = true;
        bool translated = false;
        QString untranslated_symbol = "";
        QString translated_symbol = "_";
    };

public:
    cipher(QWidget *parent = nullptr);
    ~cipher();

private slots:
    void on_textChanged();
    void solver_textChanged(const QString &text);
    void on_customContextMenuRequested(QPoint pos);

private:
    void reapply_highlighting_rules();
    void load_dictionary();
    void parse_text();
    void build_interactive_solver();
    bool CipherContainsSymbol(const QString &symbol);

    QString CurrentSymbol;
    QString CurrentWord;
    QVector<QVector<QVector<cipherobj> > > TheCipher;
    QMap<QString, QString> TheSolution;
    QScopedPointer<Ui::CipherUI> ui;
    cipherobjectmenu TheMenu;
    void UpdateCipher();
    void UpdateSolution();
    void RemoveUnusedSymbolsFromSolution();
};
#endif // CIPHER_H
