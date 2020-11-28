#pragma once
#include <QMenu>

class cipherobjectmenu : public QMenu
{
    Q_OBJECT

public:
    explicit cipherobjectmenu(QWidget *parent = nullptr);
    ~cipherobjectmenu() = default;

    QAction* get_clear_action();
    QMenu* get_word_list_menu();

private:
    QScopedPointer<QAction> ClearAction;
    QScopedPointer<QMenu> WordListMenu;
};
