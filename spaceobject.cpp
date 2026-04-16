#include "spaceobject.h"
#include <QRandomGenerator>
#include <QtMath>
#include <QPainterPath>

SpaceObject::SpaceObject(ObjectType type, QGraphicsItem *parent)
    : QGraphicsItem(parent), objType(type)
{
    setZValue(-1);
    initializeObject();

    // Enable collision detection
    if (objType == Planet || objType == Asteroid)
    {
        setFlag(QGraphicsItem::ItemIsMovable, false);
    }
}

void SpaceObject::initializeObject()
{
    QRandomGenerator *rng = QRandomGenerator::global();

    velocityX = (rng->bounded(100) - 50) / 100.0 * 0.6;  // Range: -0.3 to 0.3
    velocityY = (rng->bounded(100) - 50) / 100.0 * 0.6;  // Range: -0.3 to 0.3

    // Lifetime
    maxLifetime = rng->bounded(1800, 5400);
    lifetime = maxLifetime;

    switch(objType)
    {
    case Star:
        size = rng->bounded(1, 3);
        color = QColor(200, 200, 255, rng->bounded(100, 200));
        setZValue(-2);
        break;

    case Planet:
        size = rng->bounded(30, 80);
        {
            int colorChoice = rng->bounded(3);
            if (colorChoice == 0)
                color = QColor(80, 60, 100, 150);  // Purple
            else if (colorChoice == 1)
                color = QColor(60, 80, 100, 150);  // Blue
            else
                color = QColor(100, 70, 60, 150);  // Brown
        }
        break;

    case Asteroid:
        size = rng->bounded(10, 30);
        color = QColor(70, 70, 80, 180);

        int sides = rng->bounded(5, 8);
        for (int i = 0; i < sides; ++i)
        {
            qreal angle = (i * 360.0 / sides) + rng->bounded(-20, 20);
            qreal radius = size * (0.7 + rng->bounded(0, 60) / 100.0);
            qreal px = radius * qCos(angle * M_PI / 180);
            qreal py = radius * qSin(angle * M_PI / 180);
            asteroidShape << QPointF(px, py);
        }
        break;
    }
}

QRectF SpaceObject::boundingRect() const
{
    qreal margin = size + 2;
    return QRectF(-margin, -margin, margin * 2, margin * 2);
}

QPainterPath SpaceObject::shape() const
{
    QPainterPath path;

    if (objType == Planet)
    {
        path.addEllipse(QPointF(0, 0), size/2, size/2);
    }
    else if (objType == Asteroid)
    {
        path.addPolygon(asteroidShape);
    }
    else
    {
        path.addEllipse(QPointF(0, 0), 0, 0);
    }

    return path;
}

void SpaceObject::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    qreal fadeRatio = lifetime / maxLifetime;
    QColor fadedColor = color;
    fadedColor.setAlpha(color.alpha() * fadeRatio);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(fadedColor);

    switch(objType)
    {
    case Star:
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0, 0), size/2, size/2);
        break;

    case Planet:
        painter->setPen(QPen(QColor(100, 100, 120, 100 * fadeRatio), 1));
        painter->drawEllipse(QPointF(0, 0), size/2, size/2);
        break;

    case Asteroid:
        painter->setPen(QPen(QColor(90, 90, 100, 150 * fadeRatio), 1));
        painter->drawPolygon(asteroidShape);
        break;
    }
}

void SpaceObject::update()
{
    setPos(x() + velocityX, y() + velocityY);

    qreal halfWidth = 1000;
    qreal halfHeight = 1000;

    if (x() > halfWidth)
        setPos(-halfWidth, y());
    else if (x() < -halfWidth)
        setPos(halfWidth, y());

    if (y() > halfHeight)
        setPos(x(), -halfHeight);
    else if (y() < -halfHeight)
        setPos(x(), halfHeight);

    lifetime -= 1;
    setProperty("lifetime", lifetime);

    QGraphicsItem::update();
}
