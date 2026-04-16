#include "enemy.h"
#include <QtMath>
#include <QRandomGenerator>
#include <QPen>
#include <QTimer>

Enemy::Enemy(EnemyType type, QPointF startPos, QGraphicsItem *parent)
    : QGraphicsRectItem(parent), type(type)
{
    setPos(startPos);
    angle = 0;
    wanderTimer = 0;
    shootTimer = 0;

    switch(type)
    {
    case Chaser:
        setRect(-12, -12, 24, 24);
        setBrush(QColor(255, 50, 50));  // Red
        setPen(QPen(QColor(200, 0, 0), 2));
        health = 2;
        speed = 3.0;
        break;

    case Wanderer:
        setRect(-15, -15, 30, 30);
        setBrush(QColor(100, 255, 100));  // Green
        setPen(QPen(QColor(0, 200, 0), 2));
        health = 3;
        speed = 2.0;
        targetPos = startPos;
        break;

    case Stationary:
        setRect(-18, -18, 36, 36);
        setBrush(QColor(100, 100, 255));  // Blue
        setPen(QPen(QColor(0, 0, 200), 2));
        health = 5;
        speed = 0.0;
        break;

    case Shooter:
        setRect(-14, -14, 28, 28);
        setBrush(QColor(200, 100, 200));  // Purple
        setPen(QPen(QColor(150, 0, 150), 2));
        health = 3;
        speed = 1.5;
        break;
    case bossenemy:
        health = 3;
        speed = 1.5;
        break;
    }
}

void Enemy::updateMovement(QPointF playerPos)
{
    QPointF currentPos = pos();

    switch(type)
    {
    case Chaser:
    {
        // Move directly toward player
        qreal dx = playerPos.x() - currentPos.x();
        qreal dy = playerPos.y() - currentPos.y();
        qreal distance = qSqrt(dx * dx + dy * dy);

        if (distance > 0)
        {
            dx = (dx / distance) * speed;
            dy = (dy / distance) * speed;
            setPos(currentPos.x() + dx, currentPos.y() + dy);

            angle = qAtan2(dy, dx) * 180 / M_PI;
            setRotation(angle);
        }
        break;
    }

    case Wanderer:
    {
        // Pick new random target periodically
        wanderTimer++;
        if (wanderTimer >= 90)
        {
            QRandomGenerator *rng = QRandomGenerator::global();
            targetPos = QPointF(
                currentPos.x() + rng->bounded(-200, 200),
                currentPos.y() + rng->bounded(-200, 200)
                );
            wanderTimer = 0;
        }

        qreal dx = targetPos.x() - currentPos.x();
        qreal dy = targetPos.y() - currentPos.y();
        qreal distance = qSqrt(dx * dx + dy * dy);

        if (distance > 5)
        {
            dx = (dx / distance) * speed;
            dy = (dy / distance) * speed;
            setPos(currentPos.x() + dx, currentPos.y() + dy);

            angle = qAtan2(dy, dx) * 180 / M_PI;
            setRotation(angle);
        }
        break;
    }

    case Stationary:
    {
        qreal dx = playerPos.x() - currentPos.x();
        qreal dy = playerPos.y() - currentPos.y();
        angle = qAtan2(dy, dx) * 180 / M_PI;
        setRotation(angle);
        break;
    }

    case Shooter:
    {
        // Keep distance from player
        qreal dx = playerPos.x() - currentPos.x();
        qreal dy = playerPos.y() - currentPos.y();
        qreal distance = qSqrt(dx * dx + dy * dy);

        qreal targetDistance = 300;
        if (distance < targetDistance)
        {
            dx = -(dx / distance) * speed;
            dy = -(dy / distance) * speed;
            setPos(currentPos.x() + dx, currentPos.y() + dy);
        }
        else if (distance > targetDistance + 100)
        {
            dx = (dx / distance) * speed;
            dy = (dy / distance) * speed;
            setPos(currentPos.x() + dx, currentPos.y() + dy);
        }

        angle = qAtan2(playerPos.y() - currentPos.y(), playerPos.x() - currentPos.x()) * 180 / M_PI;
        setRotation(angle);

        shootTimer++;
        if (shootTimer >= 120)
        {
            shootTimer = 0;
            emit shootBullet(currentPos, angle, false);
        }
        break;

    }
        case bossenemy:
    {

    }
    }

    if (currentPos.x() > 1000)
        setPos(-1000, currentPos.y());
    else if (currentPos.x() < -1000)
        setPos(1000, currentPos.y());

    if (currentPos.y() > 1000)
        setPos(currentPos.x(), -1000);
    else if (currentPos.y() < -1000)
        setPos(currentPos.x(), 1000);
}

void Enemy::takeDamage(int damage)
{
    health -= damage;

    if (health > 0)
    {
        setBrush(Qt::white);
        QTimer::singleShot(50, this, [this]() {
            switch(type)
            {
            case Chaser:
                setBrush(QColor(255, 50, 50));
                break;
            case Wanderer:
                setBrush(QColor(100, 255, 100));
                break;
            case Stationary:
                setBrush(QColor(100, 100, 255));
                break;
            case Shooter:
                setBrush(QColor(200, 100, 200));                
                break;
            }


        });
    }
    else
    {
        emit destroyed();
    }
}

void Enemy::setHealth(int nouveauHealth)
{
    health = nouveauHealth;
}
void Enemy::setSpeed(qreal nouveauspeed)
{
    speed = nouveauspeed;
}

void Enemy::setShootTimer(int nouveausettimer)
{
    shootTimer = nouveausettimer;
}

void Enemy::setAngle(qreal nouveauangle)
{
    angle = nouveauangle;
}

void Enemy::setWanderTimer(int nouveauwandertimer)
{
    wanderTimer = nouveauwandertimer;
}

void Enemy::setTargetPosition(QPointF TargetPosition)
{
    targetPos = TargetPosition;
}

void Enemy::idleUpdate()
{
    if (wanderTimer < INT_MAX - 1000) wanderTimer += 1;
    if (shootTimer < INT_MAX - 1000) shootTimer += 1;

    if (type == Wanderer && (wanderTimer % 600 == 0))
    {
        QRandomGenerator *rng = QRandomGenerator::global();
        targetPos = QPointF(
            pos().x() + rng->bounded(-200, 200),
            pos().y() + rng->bounded(-200, 200)
            );
    }

    if (type == Shooter)
    {
        if (shootTimer > 1000000) shootTimer = 1000000;
    }
}
