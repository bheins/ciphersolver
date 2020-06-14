#pragma once
#include <QMenu>

class cipherobjectmenu : public QMenu
{
    Q_OBJECT

public:
    explicit cipherobjectmenu(QWidget *parent = nullptr);
    ~cipherobjectmenu() = default;
};
