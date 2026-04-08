#include "upgrade_menu.h"
#include <QKeyEvent>
#include <QApplication>
#include <QEvent>

UpgradeMenu::UpgradeMenu(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("LEVEL UP ! Choisissez un bonus");
    setFixedSize(300, 250);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog); // Fenêtre sans bordure pour le style

    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *title = new QLabel("Amélioration disponible :", this);
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    QPushButton *btnSpeed = new QPushButton(" Vitesse (+0.4)", this);
    QPushButton *btnDamage = new QPushButton(" Dégâts (+1)", this);
    QPushButton *btnHealth = new QPushButton(" Vie Max (+2)", this);

    layout->addWidget(btnSpeed);
    layout->addWidget(btnDamage);
    layout->addWidget(btnHealth);

    connect(btnSpeed, &QPushButton::clicked, [this]() { emit upgradeSelected(0); accept(); });
    connect(btnDamage, &QPushButton::clicked, [this]() { emit upgradeSelected(1); accept(); });
    connect(btnHealth, &QPushButton::clicked, [this]() { emit upgradeSelected(2); accept(); });
    this->setStyleSheet("background-color: #121212; color: white; font-family: 'Segoe UI', sans-serif;");

}
void UpgradeMenu::navigateWithJoystick(double x, double y, bool tir, bool ultimate) {

    if ((y > 0.7 || y < -0.7) && joystickAuNeutre) {
        this->focusNextChild();
        joystickAuNeutre = false;
    }

    if (y > -0.2 && y < 0.2) {
        joystickAuNeutre = true;
    }

    if (tir) {
        QWidget* boutonActuel = this->focusWidget();

        if (boutonActuel) {
            QKeyEvent pressSpace(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
            QApplication::sendEvent(boutonActuel, &pressSpace);

            QKeyEvent releaseSpace(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier);
            QApplication::sendEvent(boutonActuel, &releaseSpace);
        }
    }
}
