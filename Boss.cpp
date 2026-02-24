#include "Boss.h"
#include <QtMath>
#include <QRandomGenerator>
#include <QPen>
#include <QTimer>


Boss::Boss(BossType type, QPointF startPos, QGraphicsItem *parent)
    :type(type), Enemy(EnemyType::bossenemy,startPos)
{
    setPos(startPos);

    switch(type)
    {
    case Boss1:
    {
        setRect(-12, -12, 400, 400);
        setBrush(QColor(255, 50, 50));  // Red
        setPen(QPen(QColor(200, 0, 0), 2));
        setHealth(300);
        setSpeed(7.0);
        break;
    }

    case Boss2:
    {
        setRect(-15, -15, 300, 300);
        setBrush(QColor(100, 255, 100));  // Green
        setPen(QPen(QColor(0, 200, 0), 2));
        setHealth(200);
        setSpeed(5.0);
        setTargetPosition(startPos);
        break;
    }

    case Boss3:
    {
        setRect(-18, -18, 500, 500);
        setBrush(QColor(100, 100, 255));  // Blue
        setPen(QPen(QColor(0, 0, 200), 2));
        setHealth(500);
        setSpeed(0.0);
        break;
    }

    case Boss4:
    {
        setRect(-14, -14, 600, 600);
        setBrush(QColor(200, 100, 200));  // Purple
        setPen(QPen(QColor(150, 0, 150), 2));
        setHealth(1000);
        setSpeed(0.5);
        break;
    }
    }
}

void Boss::updateMovement(QPointF playerPos)
{
    QPointF currentPos = pos();

    switch(type)
    {
    case Boss1:
    {
        // Move directly toward player
        qreal dx = playerPos.x() - currentPos.x();
        qreal dy = playerPos.y() - currentPos.y();
        qreal distance = qSqrt(dx * dx + dy * dy);

        if (distance > 0)
        {
            dx = (dx / distance) * getSpeed();
            dy = (dy / distance) * getSpeed();
            setPos(currentPos.x() + dx, currentPos.y() + dy);

            // Rotate to face player
            setAngle( qAtan2(dy, dx) * 180 / M_PI);
            setRotation(getAngle());
        }
        break;
    }

    case Boss2:
    {
        // Pick new random target periodically
        setWanderTimer(getWanderTimer()+1);
        if (getWanderTimer() >= 180)  // Every 3 seconds
        {
            QRandomGenerator *rng = QRandomGenerator::global();
            setTargetPosition(QPointF(currentPos.x() + rng->bounded(-200, 200), currentPos.y() + rng->bounded(-200, 200)));
            setWanderTimer(0);
        }

        // Move toward target
        qreal dx = getTargetPosition().x() - currentPos.x();
        qreal dy = getTargetPosition().y() - currentPos.y();
        qreal distance = qSqrt(dx * dx + dy * dy);

        if (distance > 5)
        {
            dx = (dx / distance) * getSpeed();
            dy = (dy / distance) * getSpeed();
            setPos(currentPos.x() + dx, currentPos.y() + dy);

            setAngle(qAtan2(dy, dx) * 180 / M_PI);
            setRotation(getAngle());
        }
        break;
    }

    case Boss3:
    {
        // Rotate to face player
        qreal dx = playerPos.x() - currentPos.x();
        qreal dy = playerPos.y() - currentPos.y();
        setAngle(qAtan2(dy, dx) * 180 / M_PI);
        setRotation(getAngle());
        break;
    }

    case Boss4:
    {
        // Keep distance from player (kiting behavior)
        qreal dx = playerPos.x() - currentPos.x();
        qreal dy = playerPos.y() - currentPos.y();
        qreal distance = qSqrt(dx * dx + dy * dy);

        // Try to maintain distance of 300 units
        qreal targetDistance = 300;
        if (distance < targetDistance)
        {
            // Move away
            dx = -(dx / distance) * getSpeed();
            dy = -(dy / distance) * getSpeed();
            setPos(currentPos.x() + dx, currentPos.y() + dy);
        }
        else if (distance > targetDistance + 100)
        {
            // Move closer
            dx = (dx / distance) * getSpeed();
            dy = (dy / distance) * getSpeed();
            setPos(currentPos.x() + dx, currentPos.y() + dy);
        }

        // Always face player
        setAngle(qAtan2(playerPos.y() - currentPos.y(), playerPos.x() - currentPos.x()) * 180 / M_PI);
        setRotation(getAngle());

        // Shoot periodically
        setShootTimer(getShootTimer()+1);
        if (getShootTimer() >= 120)  // Shoot every 2 seconds
        {
            setShootTimer(0);
            emit shootBullet(currentPos, getAngle());
        }
        break;
    }
    }

    // Wrap around map edges
    if (currentPos.x() > 1000)
        setPos(-1000, currentPos.y());
    else if (currentPos.x() < -1000)
        setPos(1000, currentPos.y());

    if (currentPos.y() > 1000)
        setPos(currentPos.x(), -1000);
    else if (currentPos.y() < -1000)
        setPos(currentPos.x(), 1000);
}

void Boss::takeDamage(int damage)
{
    setHealth(getHealth()-damage);

    // Visual feedback - flash white
    if (getHealth() > 0)
    {
        setBrush(Qt::white);
        QTimer::singleShot(50, this, [this]() {
            switch(type)
            {
            case Boss1:
                setBrush(QColor(255, 50, 50));
                break;
            case Boss2:
                setBrush(QColor(100, 255, 100));
                break;
            case Boss3:
                setBrush(QColor(100, 100, 255));
                break;
            case Boss4:
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
