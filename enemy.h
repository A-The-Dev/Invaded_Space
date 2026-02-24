#ifndef ENEMY_H
#define ENEMY_H

#include <QGraphicsRectItem>
#include <QObject>
#include <QPointF>

class Enemy : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    enum EnemyType { Chaser, Wanderer, Stationary, Shooter, bossenemy };

    Enemy(EnemyType type, QPointF startPos, QGraphicsItem *parent = nullptr);

    void updateMovement(QPointF playerPos);
//les get
    EnemyType getType() const { return type; }
    int getHealth() const { return health; }
    qreal getSpeed() const {return speed; }
    qreal getAngle() const {return angle;}
    int getShootTimer() const {return shootTimer;}
    int getWanderTimer() const {return wanderTimer;}
    QPointF getTargetPosition() const {return targetPos;}
// damage
    void takeDamage(int damage);
//les set
    void setHealth(int nouveauHealth);
    void setSpeed(qreal nouveauspeed);
    void setShootTimer(int nouveausettimer);
    void setAngle(qreal nouveauangle);
    void setWanderTimer(int nouveauwandertimer);
    void setTargetPosition(QPointF TargetPosition);



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
