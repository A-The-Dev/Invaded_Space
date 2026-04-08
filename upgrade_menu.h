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
private:
    int selectedOption = 0; // 0: Vitesse, 1: Dégâts, 2: PV, etc.
    bool joystickNeutral = true;
    QList<QPushButton*> boutons;
    bool joystickAuNeutre = true;

public slots:
    void navigateWithJoystick(double x, double y, bool tir, bool ulti);
};
#endif // UPGRADE_MENU_H
