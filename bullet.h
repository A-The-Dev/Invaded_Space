#ifndef BULLET_H
#define BULLET_H

#include <QGraphicsEllipseItem>
#include <QObject>

class Bullet : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT
public:
    Bullet(QPointF startPos, qreal angle, bool fromPlayer = true,int damage = 1, QGraphicsItem *parent = nullptr);

    bool isFromPlayer() const { return fromPlayer; }

public slots:
    void move();
    int getDamage() const { return damage; }

private:
    qreal angle;
    qreal speed;
    bool fromPlayer;
    int damage;
};

#endif // BULLET_H
