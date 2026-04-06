#ifndef HUD_H
#define HUD_H

#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QObject>

class QTimer;

class HUD : public QObject
{
    Q_OBJECT
public:
    HUD(QGraphicsScene *scene, QObject *parent = nullptr);

    void updatePosition(QPointF cameraPos, QPointF playerPos);
    void updateUltimate(float percentage);
public slots:
    void updateHealth(int health, int maxHealth);
    void updateLevel(int level);
    void updateXP(int currentXP, int xpToNextLevel);

private slots:
    void onUltimatePulse();

private:
    // Player-attached UI
    QGraphicsRectItem *healthBarBackground;
    QGraphicsRectItem *healthBarFill;
    QGraphicsTextItem *healthText;

    QGraphicsRectItem *levelBackground;
    QGraphicsTextItem *levelText;

    // Player-attached XP
    QGraphicsRectItem *playerXPBackground;
    QGraphicsRectItem *playerXPFill;

    // Ultimat
    QGraphicsRectItem* ultimateBar;
    QGraphicsRectItem* ultimateBackground;

    // Pulse animation timer for ultimate full effect
    QTimer* ultimatePulseTimer;
    bool ultimatePulseGrowing = false;

    QGraphicsScene *scene;
    QPointF smoothPos;
};

#endif // HUD_H
