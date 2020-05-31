#ifndef CIPHEROBJECTMENU_H
#define CIPHEROBJECTMENU_H

#include <QMenu>

class cipherobjectmenu : public QMenu
{
    Q_OBJECT

public:
    explicit cipherobjectmenu(QWidget *parent = nullptr);
    ~cipherobjectmenu() = default;

    QAction* actionAddTranslation;
    QAction* actionRemoveTranslation;
    QAction* actionToggleTranslatable;
};

#endif // CIPHEROBJECTMENU_H
