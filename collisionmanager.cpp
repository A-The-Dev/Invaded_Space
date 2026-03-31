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

void CollisionManager::checkCollisions(Player *player, QList<Bullet*> &bullets,
                                       QList<Enemy*> &enemies, QList<Boss*> &bosses,
                                       QList<Ultimate*> &ultimates)
{
    // Check bullet-enemy/boss collisions (only player bullets)
    for (int i = 0; i < bullets.size(); ++i)
    {
        Bullet* b = bullets[i];
        if (!b->isFromPlayer())
            continue;

        bool removed = false;

        // vs enemies
        for (int j = 0; j < enemies.size(); ++j)
        {
            if (checkCollision(b, enemies[j]))
            {
                enemies[j]->takeDamage(1);

                bullets.removeAt(i);
                if (b->scene()) b->scene()->removeItem(b);
                delete b;

                removed = true;
                --i;               // so outer loop stays correct
                break;
            }
        }

        if (removed) continue;     // IMPORTANT: don't check bosses with a deleted bullet

        // vs bosses
        for (int j = 0; j < bosses.size(); ++j)
        {
            if (checkCollision(b, bosses[j]))
            {
                bosses[j]->takeDamage(1);

                bullets.removeAt(i);
                if (b->scene()) b->scene()->removeItem(b);
                delete b;

                --i;
                break;
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

            emit playerHitEnemy(enemies[i]);
        }
    }
    // Check player-enemy collisions with pushback
    for (int i = 0; i < bosses.size(); ++i)
    {
        if (checkCollision(player, bosses[i]))
        {
            // Calculate push direction (away from enemy)
            QPointF playerPos = player->pos();
            QPointF BossPos = bosses[i]->pos();
            QPointF pushDir = playerPos - BossPos;

            // Push player back
            player->pushBack(pushDir, 8.0);

            // Deal damage if not invincible
            if (!player->isInvincible())
            {
                player->takeDamage(1);
            }

            emit playerHitBoss(bosses[i]);
        }
    }
    for (int i = ultimates.size() - 1; i >= 0; --i) {
        if (checkCollision(ultimates[i], player)) {
            if (!player->isInvincible()) {
                player->takeDamage(2); // Ultimates deal double damage

                // Laser (Boss 4) doesn't disappear on hit, others do
                if (ultimates[i]->getSpeed() > 0) {
                    Ultimate* u = ultimates.takeAt(i);
                    if (u->scene()) u->scene()->removeItem(u);
                    delete u;
                }
            }
        }
    }
}
