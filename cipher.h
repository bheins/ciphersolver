#ifndef CIPHER_H
#define CIPHER_H

#include <QMainWindow>
#include <QHash>
#include <QListWidgetItem>
#include "cipherobjectmenu.h"
#include <QRecursiveMutex>
#include <QMutex>

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
    void on_textEdit_textChanged();
    void solver_textChanged(const QString &text);
    void on_cipherwordlineedit_customContextMenuRequested(QPoint pos);
    void on_wordFilter_textEdited(const QString &text);
    void on_wordSelection_itemClicked(QListWidgetItem *item);

private:
    void load_dictionary();
    void build_interactive_solver();
    void update_cipher();
    void update_solution();
    cipherobj *find_cipher_by_untranslated_symbol(const QString &symbol);
    cipherobj *find_cipher_by_translated_symbol(const QString& symbol);

    QStringList Dictionary;
    QString CurrentSymbol;
    QString CurrentWord;
    QVector<QVector<QVector<QString> > > TheCipher;
    QMap<int, cipherobj*> TheSolution;
    QScopedPointer<Ui::CipherUI> ui;
    QMutex CipherMutex;
};
#endif // CIPHER_H
