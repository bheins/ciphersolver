#include "cipherobjectmenu.h"
#include <QMenu>

cipherobjectmenu::cipherobjectmenu(QWidget *parent)
    : QMenu(parent)
    , ClearAction(new QAction("Clear"))
    , WordListMenu(new QMenu("Word List"))
{
    this->setObjectName("CipherWordMenu");
    WordListMenu->setObjectName("WordList");
    this->addAction(ClearAction.get());
    this->addSeparator();
    this->addMenu(WordListMenu.get());
}

QAction* cipherobjectmenu::get_clear_action()
{
    return ClearAction.get();
}

QMenu* cipherobjectmenu::get_word_list_menu()
{
    return WordListMenu.get();
}
