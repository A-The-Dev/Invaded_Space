#ifndef COLLISIONMANAGER_H
#define COLLISIONMANAGER_H

#include <QObject>
#include <QList>
#include "player.h"
#include "bullet.h"
#include "enemy.h"
#include "Boss.h"
#include "ultimate.h"

class CollisionManager : public QObject
{
    Q_OBJECT
public:
    CollisionManager(QObject *parent = nullptr);

    bool isOffScreen(QGraphicsItem *item, QPointF cameraCenter, qreal viewWidth, qreal viewHeight);
    void checkCollisions(Player *player, QList<Bullet*> &bullets, QList<Enemy*> &enemies, QList<Boss*> &bosses, QList<Ultimate*> &ultimates);

signals:
    void enemyDestroyed(Enemy *enemy);
    void BossDestroyed(Boss *boss);
    void playerHitEnemy(Enemy *enemy);
    void playerHitBoss(Boss *boss);

private:
    bool checkCollision(QGraphicsItem *item1, QGraphicsItem *item2);

};

#endif // COLLISIONMANAGER_H
