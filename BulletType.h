#ifndef BULLETTYPE_H
#define BULLETTYPE_H

#include <QGraphicsEllipseItem>
#include <QObject>
#include <QPainter>
#include <QPixmap>

class BulletType:  public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT
public:
    enum BulletTypes{enemy,player,Boss1,Boss2,Boss4};
    //make a way to directly call the bullet type you want
    BulletType(QPointF startPos, qreal angle, BulletTypes type = BulletType::Boss2, QGraphicsItem *parent = nullptr);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    qreal angle;
    qreal speed;
    QPixmap sprite;
    BulletTypes type;
};


#endif // BULLETTYPE_H
