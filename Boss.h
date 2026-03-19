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

signals:
    void destroyed();
    void shootBullet(QPointF position, qreal angle, bool boss);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);
private:
    BossType type;
    QPixmap sprite;

};

#endif // BOSS_H
