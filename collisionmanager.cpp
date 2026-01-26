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
    qreal margin = 150;  // Extra margin to keep bullets a bit longer

    return (itemPos.x() < cameraCenter.x() - viewWidth/2 - margin ||
            itemPos.x() > cameraCenter.x() + viewWidth/2 + margin ||
            itemPos.y() < cameraCenter.y() - viewHeight/2 - margin ||
            itemPos.y() > cameraCenter.y() + viewHeight/2 + margin);
}

void CollisionManager::checkCollisions(Player *player, QList<Bullet*> &bullets, QList<Enemy*> &enemies)
{
    // Check bullet-enemy collisions (only player bullets)
    for (int i = 0; i < bullets.size(); ++i)
    {
        if (!bullets[i]->isFromPlayer())
            continue;

        for (int j = 0; j < enemies.size(); ++j)
        {
            if (checkCollision(bullets[i], enemies[j]))
            {
                // Damage enemy
                enemies[j]->takeDamage(1);

                // Remove bullet
                if (i < bullets.size())
                {
                    Bullet *bullet = bullets[i];
                    bullets.removeAt(i);
                    if (bullet->scene())
                        bullet->scene()->removeItem(bullet);
                    delete bullet;
                    i--;
                    break;
                }
            }
        }
    }

    // Check enemy bullet-player collisions
    for (int i = 0; i < bullets.size(); ++i)
    {
        if (bullets[i]->isFromPlayer())
            continue;

        if (checkCollision(bullets[i], player))
        {
            // Damage player
            player->takeDamage(1);

            // Remove bullet
            Bullet *bullet = bullets[i];
            bullets.removeAt(i);
            if (bullet->scene())
                bullet->scene()->removeItem(bullet);
            delete bullet;
            i--;
        }
    }

    // Check player-enemy collisions with pushback
    for (int i = 0; i < enemies.size(); ++i)
    {
        if (checkCollision(player, enemies[i]))
        {
            // Calculate push direction (away from enemy)
            QPointF playerPos = player->pos();
            QPointF enemyPos = enemies[i]->pos();
            QPointF pushDir = playerPos - enemyPos;

            // Push player back
            player->pushBack(pushDir, 8.0);

            // Deal damage if not invincible
            if (!player->isInvincible())
            {
                player->takeDamage(1);
            }

            emit playerHit(enemies[i]);
        }
    }
}
