#include "collisionmanager.h"
#include <QGraphicsScene>
#include <QtMath>

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
            if (checkCollision(b, enemies[j]))
            {
                enemies[j]->takeDamage(b->getDamage());

                // Remove bullet
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
            if (checkCollision(b, bosses[j]))
            {
                bosses[j]->takeDamage(b->getDamage());

                // Remove bullet
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
        if (bullets[i]->isFromPlayer())
            continue;

        if (checkCollision(bullets[i], player))
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
        if (checkCollision(player, enemies[i]))
        {
            QPointF playerPos = player->pos();
            QPointF enemyPos = enemies[i]->pos();
            QPointF pushDir = playerPos - enemyPos;

            player->pushBack(pushDir, 8.0);

            if (!player->isInvincible())
            {
                player->takeDamage(1);
            }

            emit playerHitEnemy(enemies[i]);
        }
    }

    // Player vs bosses (contact damage)
    for (int i = 0; i < bosses.size(); ++i)
    {
        if (checkCollision(player, bosses[i]))
        {
            QPointF playerPos = player->pos();
            QPointF bossPos = bosses[i]->pos();
            QPointF pushDir = playerPos - bossPos;

            player->pushBack(pushDir, 8.0);

            if (!player->isInvincible())
            {
                player->takeDamage(1);
            }

            emit playerHitBoss(bosses[i]);
        }
    }

    // Ultimate attacks vs player
    for (int i = ultimates.size() - 1; i >= 0; --i)
    {
        if (checkCollision(ultimates[i], player))
        {
            if (!player->isInvincible())
            {
                player->takeDamage(2);

                if (ultimates[i]->getSpeed() > 0)
                {
                    Ultimate* u = ultimates.takeAt(i);
                    if (u->scene()) u->scene()->removeItem(u);
                    delete u;

                    // Don't emit with invalid boss pointer
                    // Only emit if you have a valid boss reference
                }
            }
        }
    }
}
