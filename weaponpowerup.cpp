#include "weaponpowerup.h"
#include <QBrush>
#include <QPen>
#include <QtMath>

WeaponPowerUp::WeaponPowerUp(QPointF pos, WeaponType type, QGraphicsItem *parent)
    : QGraphicsRectItem(parent), type(type) {
    setRect(-10, -10, 20, 20);
    setPos(pos);

    // Design : Carré technologique orange/rouge pour les armes
    setPen(QPen(QColor(255, 100, 0), 2));
    setBrush(QBrush(QColor(255, 50, 0, 150)));
    setZValue(45);
}

void WeaponPowerUp::moveTowardsPlayer(QPointF playerPos) {
    QPointF currentPos = pos();
    qreal dx = playerPos.x() - currentPos.x();
    qreal dy = playerPos.y() - currentPos.y();
    qreal dist = qSqrt(dx*dx + dy*dy);

    if (dist < 200) { // Rayon d'attraction
        attractionSpeed += 0.5;
        setPos(x() + (dx/dist) * attractionSpeed, y() + (dy/dist) * attractionSpeed);
    }
}
