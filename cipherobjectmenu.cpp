#include "cipherobjectmenu.h"
#include <QMenu>

cipherobjectmenu::cipherobjectmenu(QWidget *parent)
    : QMenu(parent)
    , actionAddTranslation(/*addAction("Add Translation")*/)
    , actionRemoveTranslation(/*addAction("Remove Translation")*/)
    , actionToggleTranslatable(addAction("Toggle Translatable"))
{
}
