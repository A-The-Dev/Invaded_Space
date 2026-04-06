#include "upgrade_menu.h"

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

    // Style sombre
    setStyleSheet("background-color: #1a1a2e; color: white; font-weight: bold;");
}
