#include "Grenades.h"
#include "SoundManager.h"
#include <QGraphicsScene>
#include <QtMath>

Grenades::Grenades(QPointF startPos, qreal angle, QGraphicsItem* parent)
    : QGraphicsEllipseItem(parent), angle(angle), speed(5.0), damage(40)
{
    setRect(-15, -15, 30, 30); 
    setZValue(100);
    setPos(startPos);

    sprite = QPixmap("./resources/Grenade.png");
    if (sprite.isNull()) {
        qDebug() << "Failed to load ./resources/Grenade.png";
    }
}

void Grenades::move()
{
    if (isExploding) {
        explosionTimer++;

        // Expand the explosion radius visually
        qreal grow = 4.0;
        setRect(rect().adjusted(-grow, -grow, grow, grow));

        if (explosionTimer > 30) {
            finished = true;
            this->deleteLater();
        }
        return;
    }

    qreal radians = angle * M_PI / 180.0;
    qreal dx = qCos(radians) * speed;
    qreal dy = qSin(radians) * speed;

    setPos(x() + dx, y() + dy);
    distanceTraveled += speed;


    if (distanceTraveled >= maxDistance) {
        isExploding = true;
        speed = 0;
        sprite = QPixmap("./resources/grenadeExplosion.png");
        
        // Play explosion sound when grenade starts exploding
        SoundManager::instance()->playSound(SoundManager::GrenadeExplosion);
    }
}

qreal Grenades::getSpeed() { return speed; }

void Grenades::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing, true);

    if (!sprite.isNull()) {
        painter->drawPixmap(rect().toRect(), sprite);
    }
    else {
        
        painter->setBrush(isExploding ? QColor(255, 50, 50, 150) : Qt::red);
        painter->setPen(QPen(Qt::white, 2));
        painter->drawEllipse(rect());
    }
}