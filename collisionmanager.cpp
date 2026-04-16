#include "collisionmanager.h"
#include <QGraphicsScene>
#include <QtMath>

static bool approxNearby(QGraphicsItem *a, QGraphicsItem *b, qreal extra = 4.0)
{
    QPointF pa = a->pos();
    QPointF pb = b->pos();
    qreal dx = pa.x() - pb.x();
    qreal dy = pa.y() - pb.y();
    qreal dist2 = dx*dx + dy*dy;

    QRectF ra = a->boundingRect();
    QRectF rb = b->boundingRect();
    qreal raRadius = qMax(ra.width(), ra.height()) * 0.5;
    qreal rbRadius = qMax(rb.width(), rb.height()) * 0.5;
    qreal thresh = raRadius + rbRadius + extra;
    return dist2 <= (thresh * thresh);
}

CollisionManager::CollisionManager(QObject *parent) : QObject(parent)
{
}

bool CollisionManager::checkCollision(QGraphicsItem *item1, QGraphicsItem *item2)
{
    return item1->collidesWithItem(item2);
}

bool CollisionManager::isOffScreen(QGraphicsItem *item, QPointF cameraCenter, qreal viewWidth, qreal viewHeight)
{
    QPointF itemPos = item->pos();
    qreal margin = 150;

    return (itemPos.x() < cameraCenter.x() - viewWidth/2 - margin ||
            itemPos.x() > cameraCenter.x() + viewWidth/2 + margin ||
            itemPos.y() < cameraCenter.y() - viewHeight/2 - margin ||
            itemPos.y() > cameraCenter.y() + viewHeight/2 + margin);
}

void CollisionManager::checkCollisions(Player *player, QList<Bullet*> &bullets,
                                       QList<Enemy*> &enemies, QList<Boss*> &bosses,
                                       QList<Ultimate*> &ultimates)
{
    // Player bullets vs enemies and bosses
    for (int i = bullets.size() - 1; i >= 0; --i)
    {
        Bullet* b = bullets[i];
        if (!b->isFromPlayer())
            continue;

        bool bulletRemoved = false;

        // Check collision with enemies
        for (int j = 0; j < enemies.size(); ++j)
        {
            Enemy* e = enemies[j];
            if (!e) continue;

            if (!approxNearby(b, e)) continue;

            if (checkCollision(b, e))
            {
                e->takeDamage(b->getDamage());

                bullets.removeAt(i);
                if (b->scene()) b->scene()->removeItem(b);
                delete b;

                bulletRemoved = true;
                break;
            }
        }

        if (bulletRemoved) continue;

        // Check collision with bosses
        for (int j = 0; j < bosses.size(); ++j)
        {
            Boss* bs = bosses[j];
            if (!bs) continue;

            if (!approxNearby(b, bs)) continue;

            if (checkCollision(b, bs))
            {
                bs->takeDamage(b->getDamage());

                bullets.removeAt(i);
                if (b->scene()) b->scene()->removeItem(b);
                delete b;

                break;
            }
        }
    }

    // Enemy bullets vs player
    for (int i = bullets.size() - 1; i >= 0; --i)
    {
        Bullet* bl = bullets[i];
        if (bl->isFromPlayer())
            continue;

        if (!approxNearby(bl, player)) continue;

        if (checkCollision(bl, player))
        {
            player->takeDamage(1);

            Bullet *bullet = bullets[i];
            bullets.removeAt(i);
            if (bullet->scene())
                bullet->scene()->removeItem(bullet);
            delete bullet;
        }
    }

    // Player vs enemies (contact damage)
    for (int i = 0; i < enemies.size(); ++i)
    {
        Enemy* e = enemies[i];
        if (!e) continue;

        if (!approxNearby(player, e, 8.0)) continue;

        if (checkCollision(player, e))
        {
            QPointF playerPos = player->pos();
            QPointF enemyPos = e->pos();
            QPointF pushDir = playerPos - enemyPos;

            player->pushBack(pushDir, 8.0);

            if (!player->isInvincible())
            {
                player->takeDamage(1);
            }

            emit playerHitEnemy(e);
        }
    }

    // Player vs bosses (contact damage)
    for (int i = 0; i < bosses.size(); ++i)
    {
        Boss* b = bosses[i];
        if (!b) continue;

        if (!approxNearby(player, b, 12.0)) continue;

        if (checkCollision(player, b))
        {
            QPointF playerPos = player->pos();
            QPointF bossPos = b->pos();
            QPointF pushDir = playerPos - bossPos;

            player->pushBack(pushDir, 8.0);

            if (!player->isInvincible())
            {
                player->takeDamage(1);
            }

            emit playerHitBoss(b);
        }
    }

    // Ultimate attacks vs player
    for (int i = ultimates.size() - 1; i >= 0; --i)
    {
        Ultimate* u = ultimates[i];
        if (!u) continue;

        if (!approxNearby(u, player, 8.0)) continue;

        if (checkCollision(u, player))
        {
            if (!player->isInvincible())
            {
                player->takeDamage(2);

                if (u->getSpeed() > 0)
                {
                    Ultimate* removed = ultimates.takeAt(i);
                    if (removed->scene()) removed->scene()->removeItem(removed);
                    delete removed;
                }
            }
        }
    }
}
