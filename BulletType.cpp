#include "BulletType.h"
#include <QGraphicsScene>
#include <QtMath>
#include <QPen>
#include <QRadialGradient>

BulletType::BulletType(QPointF startPos, qreal angle,BulletTypes type, QGraphicsItem *parent)
    : QGraphicsEllipseItem(parent)
{
    // Create a perfect circle
    qreal radius = 4;
    this->type = type;
    this->angle = angle;
    this->speed = 10.0;
    QTransform t;

    // Create glowing bullet effect with radial gradient centered at (0, 0)
    QLinearGradient linearGrad(QPointF(100, 100), QPointF(200, 200));

    switch (type)
    {
    case BulletType::enemy:
        setRect(-radius, -radius, radius * 2, radius * 2);
        setPen(QPen(QColor(200, 100, 200), 1));
        break;
    case BulletType::player:
        setRect(-radius, -radius, radius * 2, radius * 2);
        setPen(QPen(QColor(255, 200, 0), 1));
        break;
    case BulletType::Boss1:
        setRect(-radius, -radius, radius * 2, radius * 2);
        setPen(QPen(QColor(200, 100, 200), 1));
        break;
    case BulletType::Boss2:
        setRect(-15, -15, 100, 40);
        setZValue(100);

        sprite = QPixmap("./resources/TrumpIceBullet.png");



        if (sprite.isNull())
        {
            qDebug() << "Failed to load :/resources/TrumpIceBullet.png";
        }
        sprite = sprite.transformed(t, Qt::SmoothTransformation);
        setBrush(Qt::transparent);

        this->speed = 15.0;
        break;

    case BulletType::Boss4:
        setRect(-15, -15, 100, 10);
        setPen(QPen(QColor(200, 100, 200), 1));

        setPos(startPos);
        setRotation(angle);
        this->speed = 25.0;
        break;

    }


    setBrush(QBrush(linearGrad));
    setPos(startPos);
    setRotation(0);
}

void BulletType::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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
