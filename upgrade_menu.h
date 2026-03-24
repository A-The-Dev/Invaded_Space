#ifndef UPGRADE_MENU_H
#define UPGRADE_MENU_H

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class UpgradeMenu : public QDialog
{
    Q_OBJECT
public:
    explicit UpgradeMenu(QWidget *parent = nullptr);

signals:
    void upgradeSelected(int choice); // 0: Vitesse, 1: Dégâts, 2: Vie
};
#endif // UPGRADE_MENU_H
