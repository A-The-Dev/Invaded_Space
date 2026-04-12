#ifndef GRENADES_H
#define GRENADES_H

#include <QGraphicsEllipseItem>
#include <QObject>
#include <QPainter>
#include <QPixmap>
#include <QDebug>

class Grenades : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT
public:
    Grenades(QPointF startPos, qreal angle, QGraphicsItem* parent = nullptr);
    qreal getSpeed();
    void setDamage(int damage = 40) { this->damage = damage; }
    int getDamage() const { return damage; }
    bool getIsExploding() const { return isExploding; }
    bool isFinished() const { return finished; }

public slots:
    void move();

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    qreal angle;
    qreal speed;
    int damage;

    bool isExploding = false;
    bool finished = false;
    int explosionTimer = 0;

    qreal distanceTraveled = 0;
    qreal maxDistance = 200.0;

    QPixmap sprite;
};

#endif // GRENADES_H