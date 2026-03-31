#ifndef ULTIMATE_H
#define ULTIMATE_H


#include <QGraphicsEllipseItem>
#include <QObject>
#include <QPainter>
#include <QPixmap>
#include "Boss.h"
#include "player.h"


class Ultimate : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT
public:
    Ultimate(QPointF startPos, qreal angle, bool fromPlayer = true,
             Player* player = nullptr, Boss* boss = nullptr,
             bool fromBoss = false, QGraphicsItem *parent = nullptr);

    bool isFromPlayer() const { return fromPlayer; }
    qreal getSpeed();

public slots:
    void move();

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    qreal angle;
    qreal speed;
    bool fromPlayer;
    QPixmap sprite;
    bool fromBoss;
};
#endif // ULTIMATE_H

