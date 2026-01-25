#ifndef COLLISIONMANAGER_H
#define COLLISIONMANAGER_H

#include <QObject>
#include <QList>
#include "player.h"
#include "bullet.h"
#include "enemy.h"

class CollisionManager : public QObject
{
    Q_OBJECT
public:
    CollisionManager(QObject *parent = nullptr);

    void checkCollisions(Player *player, QList<Bullet*> &bullets, QList<Enemy*> &enemies);
    bool isOffScreen(QGraphicsItem *item, QPointF cameraCenter, qreal viewWidth, qreal viewHeight);

signals:
    void enemyDestroyed(Enemy *enemy);
    void playerHit(Enemy *enemy);

private:
    bool checkCollision(QGraphicsItem *item1, QGraphicsItem *item2);
};

#endif // COLLISIONMANAGER_H
