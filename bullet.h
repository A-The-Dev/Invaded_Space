#ifndef BULLET_H
#define BULLET_H

#include <QGraphicsEllipseItem>
#include <QObject>

class Bullet : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT
public:
    Bullet(QPointF startPos, qreal angle, bool fromPlayer = true, QGraphicsItem *parent = nullptr);

    bool isFromPlayer() const { return fromPlayer; }

public slots:
    void move();

private:
    qreal angle;
    qreal speed;
    bool fromPlayer;
};

#endif // BULLET_H
