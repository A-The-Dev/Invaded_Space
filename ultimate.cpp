#include "ultimate.h"
#include "player.h"  
#include "boss.h"
#include <QtMath>
#include <QRadialGradient>

Ultimate::Ultimate(QPointF startPos, qreal angle, bool fromPlayer, Player* player, Boss* boss, bool fromBoss, QGraphicsItem *parent)
    : QGraphicsEllipseItem(parent), fromPlayer(fromPlayer), fromBoss(fromBoss), angle(angle)
{

    setPos(startPos);
    setRotation(angle);
    setZValue(101);

    if (fromBoss && boss) {
        switch(boss->getType()) {
        case Boss::Boss1:
            setRect(-15, -15, 30, 30);
            this->speed = 7.0;
            break;

        case Boss::Boss2:
            setRect(-50, -15, 100, 30);
            this->speed = 1.0;
            setBrush(QColor(100, 200, 255));
            break;
        case Boss::Boss4:
            setRect(0, -30, 2000, 60);
            this->speed = 0;
            break;
        }
    }
    else if (fromPlayer)
    {
        setRect(-10, -10, 20, 20);
        this->speed = 15.0;
    }
}

void Ultimate::move()
{
    // Standard linear movement based on angle
    qreal radians = angle * M_PI / 180.0;
    qreal dx = qCos(radians) * speed;
    qreal dy = qSin(radians) * speed;

    setPos(x() + dx, y() + dy);

    // Only wrap/delete if it's not the Boss 4 Laser (which is static)
    if (speed > 0) {
        QPointF currentPos = pos();
        if (qAbs(currentPos.x()) > 1500 || qAbs(currentPos.y()) > 1500) {
        }
    }
}

void Ultimate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);

    if (fromBoss)
    {
        // Boss 4
        if (rect().width() > 500) {
            QLinearGradient beamGrad(rect().topLeft(), rect().bottomLeft());
            beamGrad.setColorAt(0, QColor(255, 0, 0, 150)); // Red glow
            beamGrad.setColorAt(0.5, Qt::white);           // White core
            beamGrad.setColorAt(1, QColor(255, 0, 0, 150)); // Red glow

            painter->setPen(Qt::NoPen);
            painter->setBrush(beamGrad);
            painter->drawRect(rect());
        }
        // Boss 1 & 2
        else {
            QRadialGradient glow(0, 0, rect().width() / 2);
            glow.setColorAt(0, Qt::white);
            glow.setColorAt(0.4, QColor(200, 100, 255)); // Purple/Magenta
            glow.setColorAt(1, Qt::transparent);

            painter->setPen(Qt::NoPen);
            painter->setBrush(glow);
            painter->drawEllipse(rect());
        }
    }
}

qreal Ultimate::getSpeed() {
    return speed;
}
