#include "xporb.h"
#include <QtMath>
#include <QRadialGradient>
#include <qpen.h>

XPOrb::XPOrb(QPointF position, int xpValue, QGraphicsItem *parent)
    : QGraphicsEllipseItem(parent), xpValue(xpValue)
{
    qreal size = 8;
    setRect(-size/2, -size/2, size, size);

    // Create glowing XP orb effect
    QRadialGradient gradient(0, 0, size/2);
    gradient.setColorAt(0, QColor(100, 255, 255));  // Cyan center
    gradient.setColorAt(0.5, QColor(50, 200, 255)); // Blue
    gradient.setColorAt(1, QColor(0, 150, 255));    // Dark blue edge

    setBrush(QBrush(gradient));
    setPen(QPen(QColor(150, 255, 255), 1));

    setPos(position);
    setZValue(50);
    attractionSpeed = 0;
}

void XPOrb::moveTowardsPlayer(QPointF playerPos)
{
    QPointF currentPos = pos();
    qreal dx = playerPos.x() - currentPos.x();
    qreal dy = playerPos.y() - currentPos.y();
    qreal distance = qSqrt(dx * dx + dy * dy);

    // Start attracting when player is within 150 units
    if (distance < 150)
    {
        attractionSpeed = qMin(attractionSpeed + 0.5, 8.0);

        if (distance > 1)
        {
            qreal moveX = (dx / distance) * attractionSpeed;
            qreal moveY = (dy / distance) * attractionSpeed;
            setPos(currentPos.x() + moveX, currentPos.y() + moveY);
        }
    }
}
