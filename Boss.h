#ifndef BOSS_H
#define BOSS_H


#include <QGraphicsRectItem>
#include <QObject>
#include <QPointF>
#include "enemy.h"

class Boss : public Enemy
{
    Q_OBJECT
public:
    enum BossType { Boss1, Boss2, Boss3, Boss4 };

    Boss(BossType type, QPointF startPos, QGraphicsItem *parent = nullptr);

    void updateMovement(QPointF playerPos);
    BossType getType() const { return type; }
    void takeDamage(int damage);

signals:
    void destroyed();
    void shootBullet(QPointF position, qreal angle);

private:
    BossType type;
};


#endif // BOSS_H
