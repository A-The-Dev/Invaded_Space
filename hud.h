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
    void updateGrenades(int count);
    void setMaxGrenades(int maxGrenadeCount);

private slots:
    void onUltimatePulse();
    void onGrenadeAnimationTick();

private:
    void createGrenadeSlots();

    // Player-attached UI
    QGraphicsRectItem *healthBarBackground;
    QGraphicsRectItem *healthBarFill;
    QGraphicsTextItem *healthText;

    QGraphicsRectItem *levelBackground;
    QGraphicsTextItem *levelText;

    // Player-attached XP
    QGraphicsRectItem *playerXPBackground;
    QGraphicsRectItem *playerXPFill;

    // Grenades
    QList<QGraphicsRectItem*> grenadeSegments;
    QList<QGraphicsRectItem*> grenadeFills;
    QList<float> grenadeFillProgress;
    int maxGrenades = 0;
    int currentGrenades = 0;
    int regeneratingGrenadeIndex = -1;
    float grenadeRegenProgress = 0.0f;
    
    // Ultimate
    QGraphicsRectItem* ultimateBar;
    QGraphicsRectItem* ultimateBackground;

    // Animation timer for ultimate
    QTimer* ultimatePulseTimer;
    bool ultimatePulseGrowing = false;
    
    // Grenade fill animation timer
    QTimer* grenadeAnimationTimer;

    QGraphicsScene *scene;
    QPointF smoothPos;
};

#endif // HUD_H
