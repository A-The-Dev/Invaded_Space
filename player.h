#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsRectItem>
#include <QObject>
#include <QPointF>
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QElapsedTimer>
#include <QList>
#include "bullet.h"

class Enemy;
class Boss;

class Player : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    Player(QGraphicsItem *parent = nullptr);

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    void updateMovement();
    void updateRotation(QPointF velocity);
    qreal getAngle() const { return angle; }

    int getHealth() const { return health; }
    int getMaxHealth() const { return maxHealth; }
    void takeDamage(int damage);
    void refillHealth();
    void increaseMaxHealth(int amount);
    void pushBack(QPointF direction, qreal force);
    bool isInvincible() const { return invincibilityFrames > 0; }
    void updateInvincibility();
    void gainUltimateCharge(float amount);
    float getUltimatePercentage() const { return ultimateCharge / maxUltimateCharge; }
    bool tryUseUltimate();
    int getAttackDamage() const { return attackDamage; }
    void setAttackDamage(int val) { attackDamage = val; }

    void updateFromJoystick(qreal axisX, qreal axisY, bool isShooting);
    void shoot();

    qreal getSpeed() const { return speed; }
    void setSpeed(qreal s);

    // Toggle target selection mode (nearest vs most HP)
    void toggleTargetMode() { targetByHP = !targetByHP; }
    bool isTargetByHP() const { return targetByHP; }

    // Provide access to game enemy lists so player can target reliably
    void setEnemyLists(const QList<Enemy*> *enemiesList, const QList<Boss*> *bossesList);

    // Reset movement/input state (call when focus lost / modal shown)
    void resetInputStates();

public slots:
    void update();

signals:
    void healthChanged(int health, int maxHealth);
    void died();
    void bulletFired(Bullet *bullet);

private:
    QPixmap sprite;
    bool wPressed, aPressed, sPressed, dPressed, fPressed;
    qreal angle;             // current rotation in degrees
    qreal desiredAngle;      // target rotation in degrees
    qreal rotationSpeedCurrent; // current degrees-per-frame rotation speed
    qreal rotationSpeedDefault; // default smoothing speed
    qreal aimRotationSpeed;     // faster rotation when aiming at a target

    qreal speed;
    qreal maxSpeed;
    int health;
    int maxHealth;
    QPointF knockbackVelocity;
    int invincibilityFrames;
    float ultimateCharge = 0.0;
    const float maxUltimateCharge = 100.0;
    bool isUltimateReady = false;
    int attackDamage = 1;
    bool useJoystick = false;
    QElapsedTimer lastShotTimer;
    int msBetweenShots = 200; // 200ms = 5 balles par seconde max

    // New: toggle between nearest target and highest-HP target
    bool targetByHP = false;

    // Pointers to the Game lists (not owned)
    const QList<Enemy*> *gameEnemies = nullptr;
    const QList<Boss*>  *gameBosses  = nullptr;

    // Hold rotation toward target for a short time after shooting so movement doesn't immediately override it
    int aimHoldFrames = 0;

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);
};

#endif // PLAYER_H
