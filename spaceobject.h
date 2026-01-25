#ifndef SPACEOBJECT_H
#define SPACEOBJECT_H

#include <QGraphicsItem>
#include <QObject>
#include <QPainter>

class SpaceObject : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    enum ObjectType { Star, Planet, Asteroid };

    SpaceObject(ObjectType type, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    bool hasCollision() const { return objType == Planet || objType == Asteroid; }
    QPainterPath shape() const override;

public slots:
    void update();

private:
    ObjectType objType;
    qreal size;
    qreal velocityX;
    qreal velocityY;
    qreal lifetime;
    qreal maxLifetime;
    QColor color;
    QPolygonF asteroidShape;

    void initializeObject();
};

#endif // SPACEOBJECT_H
