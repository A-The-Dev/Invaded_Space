#include "Boss.h"
#include <QtMath>
#include <QRandomGenerator>
#include <QPen>
#include <QTimer>


Boss::Boss(BossType type, QPointF startPos, QGraphicsItem *parent)
    :type(type), Enemy(EnemyType::bossenemy,startPos)
{
    ultimateTimer = 0;
    setPos(startPos);

    switch(type)
    {
    case Boss1:
    {
        setRect(-100, -200, 100, 300);
        setZValue(100);

        sprite = QPixmap("./resources/DragonBossChaser.png");
        if (sprite.isNull())
        {
            qDebug() << "Failed to load :/resources/DragonBossChaser.png";
        }

        QTransform t;
        t.rotate(90);
        sprite = sprite.transformed(t, Qt::SmoothTransformation);
        setHealth(300);
        setSpeed(5.5);
        break;
    }

    case Boss2:
    {
        setRect(-15, -15, 300, 300);
        setZValue(100);

        sprite = QPixmap("./resources/trump.png");
        if (sprite.isNull())
        {
            qDebug() << "Failed to load :/resources/DragonBossChaser.png";
        }

        QTransform t;
        t.rotate(90);
        sprite = sprite.transformed(t, Qt::SmoothTransformation);

        setHealth(500);
        setSpeed(3.0);
        setTargetPosition(startPos);
        break;
    }
    case Boss4:
    {
        setRect(-300, -300, 600, 600);
        setZValue(100);

        sprite = QPixmap("./resources/DeathStarBoss.png");
        if (sprite.isNull())
        {
            qDebug() << "Failed to load DeathStarBoss.png";
        }

        QTransform t;
        t.rotate(90);
        sprite = sprite.transformed(t, Qt::SmoothTransformation);


        setHealth(1000);
        setSpeed(1.0);
        break;
    }
    }
}

void Boss::setUltimateTimer(int nouveausettimer)
{
    ultimateTimer = nouveausettimer;
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

            setAngle( qAtan2(dy, dx) * 90 / M_PI);
            setRotation(getAngle());
        }
        setUltimateTimer(getUltimateTimer()+1);
        if (ultimateTimer >= 300) {
            emit UseUltimate(this->pos(), this->rotation(), true);
            ultimateTimer = 0;
        }
        break;
    }

    case Boss2:
    {
        // Keep distance from player
        qreal dx = playerPos.x() - currentPos.x();
        qreal dy = playerPos.y() - currentPos.y();
        qreal distance = qSqrt(dx * dx + dy * dy);

        qreal targetDistance = 150;
        if (distance < targetDistance)
        {
            dx = -(dx / distance) * getSpeed();
            dy = -(dy / distance) * getSpeed();
            setPos(currentPos.x() + dx, currentPos.y() + dy);
        }
        else if (distance > targetDistance + 100)
        {
            dx = (dx / distance) * getSpeed();
            dy = (dy / distance) * getSpeed();
            setPos(currentPos.x() + dx, currentPos.y() + dy);
        }

        setAngle(qAtan2(playerPos.y() - currentPos.y(), playerPos.x() - currentPos.x()) * 180 / M_PI);
        setRotation(getAngle());

        setShootTimer(getShootTimer()+1);
        if (getShootTimer() >= 30)
        {
            setShootTimer(0);
            emit shootBullet(currentPos, getAngle(), true);
        }
        setUltimateTimer(getUltimateTimer()+1);
        if (ultimateTimer >= 300) {
            emit UseUltimate(this->pos(), this->rotation(), true);
            ultimateTimer = 0;
        }
        break;
    }
    case Boss4:
    {
        // Keep distance from player
        qreal dx = playerPos.x() - currentPos.x();
        qreal dy = playerPos.y() - currentPos.y();
        qreal distance = qSqrt(dx * dx + dy * dy);

        qreal targetDistance = 300;
        if (distance < targetDistance)
        {
            dx = -(dx / distance) * getSpeed();
            dy = -(dy / distance) * getSpeed();
            setPos(currentPos.x() + dx, currentPos.y() + dy);
        }
        else if (distance > targetDistance + 100)
        {
            dx = (dx / distance) * getSpeed();
            dy = (dy / distance) * getSpeed();
            setPos(currentPos.x() + dx, currentPos.y() + dy);
        }

        setAngle(qAtan2(playerPos.y() - currentPos.y(), playerPos.x() - currentPos.x()) * 180 / M_PI);
        setRotation(getAngle());

        setShootTimer(getShootTimer()+1);
        if (getShootTimer() >= 120)
        {
            setShootTimer(0);
            emit shootBullet(currentPos, getAngle(), true);
        }
        setUltimateTimer(getUltimateTimer()+1);
        if (ultimateTimer >= 300) {
            emit UseUltimate(this->pos(), this->rotation(), true);
            ultimateTimer = 0;
        }
        break;


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

void Boss::takeDamage(int damage)
{
    setHealth(getHealth()-damage);

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
void Boss::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (type == Boss4)
    {
        QPainterPath path;
        path.addEllipse(rect());
        painter->setClipPath(path);

        if (!sprite.isNull()) {
            painter->drawPixmap(rect().toRect(), sprite);
        } else {
            painter->setBrush(Qt::red);
            painter->setPen(QPen(Qt::white, 3));
            painter->drawEllipse(rect());
        }
    }
    else
    {
        if (!sprite.isNull()) {
            painter->drawPixmap(rect().toRect(), sprite);
        } else {
            painter->setBrush(Qt::red);
            painter->setPen(QPen(Qt::white, 3));
            painter->drawRect(rect());
        }
    }
}
