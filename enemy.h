#ifndef ENEMY_H
#define ENEMY_H

#include <QGraphicsRectItem>
#include <QObject>
#include <QPointF>

class Enemy : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    enum EnemyType { Chaser, Wanderer, Stationary, Shooter };

    Enemy(EnemyType type, QPointF startPos, QGraphicsItem *parent = nullptr);

    void updateMovement(QPointF playerPos);
    EnemyType getType() const { return type; }
    int getHealth() const { return health; }
    void takeDamage(int damage);

signals:
    void destroyed();
    void shootBullet(QPointF position, qreal angle);

private:
    EnemyType type;
    int health;
    qreal speed;
    qreal angle;
    QPointF targetPos;
    int wanderTimer;
    int shootTimer;
};

#endif // ENEMY_H
