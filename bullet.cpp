#include "bullet.h"
#include <QGraphicsScene>
#include <QtMath>
#include <QPen>
#include <QRadialGradient>

Bullet::Bullet(QPointF startPos, qreal angle, bool fromPlayer, BossType type, bool fromBoss,  QGraphicsItem *parent)
    : QGraphicsEllipseItem(parent), fromPlayer(fromPlayer)
{
    // Create a perfect circle
    qreal radius = 4;
    this->type = type;
    setDamage(1);

    // Create glowing bullet effect with radial gradient centered at (0, 0)
    QLinearGradient linearGrad(QPointF(100, 100), QPointF(200, 200));

    if (fromPlayer)
    {
        // Player bullets: Yellow/Orange/Red
        setRect(-radius, -radius, radius * 2, radius * 2);
        setPen(QPen(QColor(255, 200, 0), 1));
        this->angle = angle;
        this->speed = 10.0;
    }
    else if(fromBoss)
    {
        if(type == Boss1)
        {
            setRect(-radius, -radius, radius * 2, radius * 2);
            setPen(QPen(QColor(200, 100, 200), 1));
            this->angle = angle;
            this->speed = 10.0;
        }
        else if(type == Boss2)
        {
            setRect(-15, -15, 100, 40);
            setZValue(100);

            sprite = QPixmap("../../resources/TrumpIceBullet.png");

            QTransform t;

            if (sprite.isNull())
            {
                qDebug() << "Failed to load :/resources/TrumpIceBullet.png";
            }
            sprite = sprite.transformed(t, Qt::SmoothTransformation);
            setBrush(Qt::transparent);
            this->angle = angle;
            this->speed = 15.0;
        }
        else if( type == Boss4)
        {

            setRect(-15, -15, 100, 10);
            setPen(QPen(QColor(200, 100, 200), 1));

            setPos(startPos);
            setRotation(angle);
            this->angle = angle;
            this->speed = 25.0;
        }
    }
    else if(fromBoss)
    {
        if(type == Boss1)
        {
            setRect(-radius, -radius, radius * 2, radius * 2);
            setPen(QPen(QColor(200, 100, 200), 1));
            this->angle = angle;
            this->speed = 10.0;
        }
        else if(type == Boss2)
        {
            setRect(-15, -15, 100, 40);
            setZValue(100);

            sprite = QPixmap("../../resources/TrumpIceBullet.png");

            QTransform t;

            if (sprite.isNull())
            {
                qDebug() << "Failed to load :/resources/TrumpIceBullet.png";
            }
            sprite = sprite.transformed(t, Qt::SmoothTransformation);
            setBrush(Qt::transparent);
            this->angle = angle;
            this->speed = 15.0;
        }
        else if( type == Boss4)
        {

            setRect(-15, -15, 100, 10);
            setPen(QPen(QColor(200, 100, 200), 1));

            setPos(startPos);
            setRotation(angle);
            this->angle = angle;
            this->speed = 25.0;
        }
    }
    else
    {
        // Enemy bullets: Purple/Magenta
        setRect(-radius, -radius, radius * 2, radius * 2);
        setPen(QPen(QColor(200, 100, 200), 1));
        this->angle = angle;
        this->speed = 10.0;
    }

    setBrush(QBrush(linearGrad));
    setPos(startPos);
    setRotation(0);
}

void Bullet::move()
{
    qreal radians = angle * M_PI / 180;
    qreal dx = qCos(radians) * speed;
    qreal dy = qSin(radians) * speed;

    setPos(x() + dx, y() + dy);

    // Wrap around map edges
    QPointF currentPos = pos();
    qreal mapWidth = 2000;
    qreal mapHeight = 2000;
    qreal halfWidth = mapWidth / 2;
    qreal halfHeight = mapHeight / 2;

    if (currentPos.x() > halfWidth)
        setPos(-halfWidth, currentPos.y());
    else if (currentPos.x() < -halfWidth)
        setPos(halfWidth, currentPos.y());

    if (currentPos.y() > halfHeight)
        setPos(currentPos.x(), -halfHeight);
    else if (currentPos.y() < -halfHeight)
        setPos(currentPos.x(), halfHeight);
}

qreal Bullet::getSpeed()
{
    return speed;
}

void Bullet::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (this->type == Boss2)
    {
        //Si boss 2
        QPainterPath path;
        path.addEllipse(rect());
        painter->setClipPath(path);

        if (!sprite.isNull()) {
            painter->drawPixmap(rect().toRect(), sprite);
        } else {
            // Si image ne load pas bien
            painter->setBrush(Qt::red);
            painter->setPen(QPen(Qt::white, 3));
            painter->drawEllipse(rect());
        }
    }
    else
    {
        //fait les bullet normal
        painter->setPen(this->pen());
        painter->setBrush(this->brush());

        painter->drawEllipse(rect());
    }
}
