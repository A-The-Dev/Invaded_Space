#ifndef XPORB_H
#define XPORB_H

#include <QGraphicsEllipseItem>
#include <QObject>

class XPOrb : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT
public:
    XPOrb(QPointF position, int xpValue, QGraphicsItem *parent = nullptr);

    int getXPValue() const { return xpValue; }
    void moveTowardsPlayer(QPointF playerPos);

private:
    int xpValue;
    qreal attractionSpeed;
};

#endif // XPORB_H
