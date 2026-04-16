#include "xporb.h"
#include <QtMath>
#include <QRadialGradient>
#include <qpen.h>

static qreal computeSizeForXP(int xp)
{
    if (xp <= 20)
        return 8.0;

    qreal base = 8.0;
    qreal size = base + qLn(static_cast<qreal>(xp) + 1.0) * 3.0;
    return qBound(8.0, size, 36.0);
}

XPOrb::XPOrb(QPointF position, int xpValue, QGraphicsItem *parent)
    : QGraphicsEllipseItem(parent), xpValue(xpValue)
{
    qreal size = computeSizeForXP(xpValue);
    setRect(-size/2, -size/2, size, size);

    QRadialGradient gradient(0, 0, size/2);
    gradient.setColorAt(0, QColor(100, 255, 255));
    gradient.setColorAt(0.5, QColor(50, 200, 255));
    gradient.setColorAt(1, QColor(0, 150, 255));

    setBrush(QBrush(gradient));
    setPen(QPen(QColor(150, 255, 255), 1));

    setPos(position);
    setZValue(50);
    attractionSpeed = 0;
    merging = false;
    lockedFlag = false;
}

void XPOrb::moveTowardsPlayer(QPointF playerPos)
{
    QPointF currentPos = pos();
    qreal dx = playerPos.x() - currentPos.x();
    qreal dy = playerPos.y() - currentPos.y();
    qreal distance = qSqrt(dx * dx + dy * dy);

    if (merging) {
        qreal mdx = merge_target.x() - currentPos.x();
        qreal mdy = merge_target.y() - currentPos.y();
        qreal mdist = qSqrt(mdx*mdx + mdy*mdy);
        if (mdist > 0.5) {
            qreal speed = qMin(attractionSpeed + 0.6, 10.0);
            attractionSpeed = speed;
            qreal moveX = (mdx / mdist) * speed;
            qreal moveY = (mdy / mdist) * speed;
            setPos(currentPos.x() + moveX, currentPos.y() + moveY);
        }
        return;
    }

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

void XPOrb::startMerging(const QPointF &target)
{
    merging = true;
    merge_target = target;
    attractionSpeed = qMax(attractionSpeed, 2.0);
}

void XPOrb::stopMerging()
{
    merging = false;
    attractionSpeed = 0;
}
