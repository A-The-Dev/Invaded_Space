#include "bullet.h"
#include <QGraphicsScene>
#include <QtMath>
#include <QPen>
#include <QRadialGradient>

Bullet::Bullet(QPointF startPos, qreal angle, bool fromPlayer, QGraphicsItem *parent)
    : QGraphicsEllipseItem(parent), fromPlayer(fromPlayer)
{
    // Create a perfect circle
    qreal radius = 4;
    setRect(-radius, -radius, radius * 2, radius * 2);

    // Create glowing bullet effect with radial gradient centered at (0, 0)
    QRadialGradient gradient(0, 0, radius);

    if (fromPlayer)
    {
        // Player bullets: Yellow/Orange/Red
        gradient.setColorAt(0, QColor(255, 255, 100));
        gradient.setColorAt(0.5, QColor(255, 150, 0));
        gradient.setColorAt(1, QColor(255, 50, 0));
        setPen(QPen(QColor(255, 200, 0), 1));
    }
    else
    {
        // Enemy bullets: Purple/Magenta
        gradient.setColorAt(0, QColor(255, 100, 255));
        gradient.setColorAt(0.5, QColor(200, 50, 200));
        gradient.setColorAt(1, QColor(150, 0, 150));
        setPen(QPen(QColor(200, 100, 200), 1));
    }

    setBrush(QBrush(gradient));
    setPos(startPos);
    setRotation(0);
    this->angle = angle;
    this->speed = 10.0;
}

void Bullet::move()
{
    qreal radians = angle * M_PI / 180;
    qreal dx = qCos(radians) * speed;
    qreal dy = qSin(radians) * speed;

    setPos(x() + dx, y() + dy);

    // Wrap around map edges
    QPointF currentPos = pos();
    qreal mapWidth = 2000;
    qreal mapHeight = 2000;
    qreal halfWidth = mapWidth / 2;
    qreal halfHeight = mapHeight / 2;

    if (currentPos.x() > halfWidth)
        setPos(-halfWidth, currentPos.y());
    else if (currentPos.x() < -halfWidth)
        setPos(halfWidth, currentPos.y());

    if (currentPos.y() > halfHeight)
        setPos(currentPos.x(), -halfHeight);
    else if (currentPos.y() < -halfHeight)
        setPos(currentPos.x(), halfHeight);
}
