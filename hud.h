#ifndef HUD_H
#define HUD_H

#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QObject>

class HUD : public QObject
{
    Q_OBJECT
public:
    HUD(QGraphicsScene *scene, QObject *parent = nullptr);

    void updatePosition(QPointF cameraPos);

public slots:
    void updateHealth(int health, int maxHealth);
    void updateLevel(int level);
    void updateXP(int currentXP, int xpToNextLevel);

private:
    QGraphicsRectItem *healthBarBackground;
    QGraphicsRectItem *healthBarFill;
    QGraphicsTextItem *healthText;
    QGraphicsRectItem *xpBarBackground;
    QGraphicsRectItem *xpBarFill;
    QGraphicsTextItem *levelText;
    QGraphicsScene *scene;
    QPointF smoothPos;
};

#endif // HUD_H
