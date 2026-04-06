#ifndef XPORB_H
#define XPORB_H

#include <QGraphicsEllipseItem>
#include <QObject>
#include <QPointF>

class XPOrb : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT
public:
    XPOrb(QPointF position, int xpValue, QGraphicsItem *parent = nullptr);

    int getXPValue() const { return xpValue; }
    void moveTowardsPlayer(QPointF playerPos);

    void startMerging(const QPointF &target);
    void stopMerging();
    bool isMerging() const { return merging; }
    const QPointF &mergeTarget() const { return merge_target; }

    void setLocked(bool locked) { lockedFlag = locked; }
    bool isLocked() const { return lockedFlag; }

private:
    int xpValue;
    qreal attractionSpeed;

    bool merging = false;
    QPointF merge_target;
    bool lockedFlag = false;
};

#endif // XPORB_H
