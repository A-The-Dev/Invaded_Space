#include "hud.h"
#include <QFont>
#include <QBrush>
#include <QPen>

HUD::HUD(QGraphicsScene *scene, QObject *parent) : QObject(parent), scene(scene)
{
    // Create health bar background
    healthBarBackground = new QGraphicsRectItem();
    healthBarBackground->setRect(0, 0, 200, 20);
    healthBarBackground->setBrush(QBrush(QColor(50, 50, 50, 200)));
    healthBarBackground->setPen(QPen(QColor(255, 255, 255), 2));
    healthBarBackground->setZValue(100);
    scene->addItem(healthBarBackground);

    // Create health bar fill
    healthBarFill = new QGraphicsRectItem();
    healthBarFill->setRect(0, 0, 200, 20);
    healthBarFill->setBrush(QBrush(QColor(50, 255, 50)));
    healthBarFill->setPen(Qt::NoPen);
    healthBarFill->setZValue(101);
    scene->addItem(healthBarFill);

    // Create health text
    healthText = new QGraphicsTextItem();
    healthText->setPlainText("HP: 10/10");
    healthText->setDefaultTextColor(Qt::white);
    QFont font("Arial", 12, QFont::Bold);
    healthText->setFont(font);
    healthText->setZValue(102);
    scene->addItem(healthText);

    // Create XP bar background
    xpBarBackground = new QGraphicsRectItem();
    xpBarBackground->setRect(0, 0, 200, 15);
    xpBarBackground->setBrush(QBrush(QColor(50, 50, 50, 200)));
    xpBarBackground->setPen(QPen(QColor(255, 255, 255), 2));
    xpBarBackground->setZValue(100);
    scene->addItem(xpBarBackground);

    // Create XP bar fill
    xpBarFill = new QGraphicsRectItem();
    xpBarFill->setRect(0, 0, 0, 15);
    xpBarFill->setBrush(QBrush(QColor(100, 200, 255)));
    xpBarFill->setPen(Qt::NoPen);
    xpBarFill->setZValue(101);
    scene->addItem(xpBarFill);

    // Create level text
    levelText = new QGraphicsTextItem();
    levelText->setPlainText("Level 1");
    levelText->setDefaultTextColor(Qt::white);
    levelText->setFont(font);
    levelText->setZValue(102);
    scene->addItem(levelText);
}

void HUD::updatePosition(QPointF cameraPos)
{
    // Position HUD elements relative to camera (top-left corner)
    qreal leftX = cameraPos.x() - 400 + 20;  // 20px from left edge
    qreal topY = cameraPos.y() - 300 + 20;   // 20px from top edge

    healthBarBackground->setPos(leftX, topY);
    healthBarFill->setPos(leftX, topY);
    healthText->setPos(leftX + 205, topY);

    // Position XP bar below health bar
    xpBarBackground->setPos(leftX, topY + 30);
    xpBarFill->setPos(leftX, topY + 30);
    levelText->setPos(leftX + 205, topY + 27);
}

void HUD::updateHealth(int health, int maxHealth)
{
    // Update health bar width
    qreal healthPercent = static_cast<qreal>(health) / maxHealth;
    healthBarFill->setRect(0, 0, 200 * healthPercent, 20);

    // Change color based on health
    if (healthPercent > 0.6)
        healthBarFill->setBrush(QBrush(QColor(50, 255, 50)));  // Green
    else if (healthPercent > 0.3)
        healthBarFill->setBrush(QBrush(QColor(255, 200, 50)));  // Yellow
    else
        healthBarFill->setBrush(QBrush(QColor(255, 50, 50)));  // Red

    // Update text
    healthText->setPlainText(QString("HP: %1/%2").arg(health).arg(maxHealth));
}

void HUD::updateLevel(int level)
{
    levelText->setPlainText(QString("Level %1").arg(level));
}

void HUD::updateXP(int currentXP, int xpToNextLevel)
{
    // Update XP bar width
    qreal xpPercent = static_cast<qreal>(currentXP) / xpToNextLevel;
    xpBarFill->setRect(0, 0, 200 * xpPercent, 15);
}
