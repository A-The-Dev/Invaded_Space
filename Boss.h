#ifndef BOSS_H
#define BOSS_H

#include <QObject>
#include <QPointF>
#include <QPainter>
#include <QPixmap>
#include "enemy.h"

class Boss : public Enemy
{
    Q_OBJECT
public:
    enum BossType { Boss1, Boss2, Boss4 };

    Boss(BossType type, QPointF startPos, QGraphicsItem *parent = nullptr);

    void updateMovement(QPointF playerPos);
    BossType getType() const { return type; }
    void takeDamage(int damage);
    int getBoss1MaxH() const { return boss1MaxHealth; }
    int getBoss2MaxH() const { return boss2MaxHealth; }
    int getBoss4MaxH() const { return boss4MaxHealth; }
    int getUltimateTimer() const {return ultimateTimer;}
    void setUltimateTimer(int nouveausettimer);


signals:
    void destroyed();
    void shootBullet(QPointF position, qreal angle, bool boss);
    void UseUltimate(QPointF position, qreal angle, bool boss);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);
private:
    BossType type;
    QPixmap sprite;
    int boss1MaxHealth = 300;
    int boss2MaxHealth = 500;
    int boss4MaxHealth = 1000;
    int ultimateTimer;

};

#endif // BOSS_H
