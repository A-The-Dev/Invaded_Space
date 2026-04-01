#ifndef BULLET_H
#define BULLET_H

#include <QGraphicsEllipseItem>
#include <QObject>
#include <QPainter>
#include <QPixmap>
#include <QPainter>
#include <QPixmap>

class Bullet : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT
public:
    enum BossType { Boss1, Boss2, Boss4 };
    Bullet( QPointF startPos, qreal angle, bool fromPlayer = true,BossType type = Boss4, bool fromBoss = false, QGraphicsItem *parent = nullptr);
    bool isFromPlayer() const { return fromPlayer; }
    qreal getSpeed();
    void setDamage(int damage = 1) {this->damage = damage;}

public slots:
    void move();
    int getDamage() const { return damage; }

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);


private:
    qreal angle;
    qreal speed;
    bool fromPlayer;
    BossType type;
    QPixmap sprite;
    bool fromBoss;

    int damage;
};

#endif // BULLET_H
