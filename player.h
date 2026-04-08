#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsRectItem>
#include <QObject>
#include <QPointF>
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QElapsedTimer>
#include "bullet.h"
#include "ultimate.h"
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
    bool getIsUltimateReady() const { return isUltimateReady; }
    //void updateFromJoystick(double angle, double vitesse, bool tir);
    void processMovement();
    void updateFromJoystick(double axisX, double axisY, bool tir, bool ulti);
    void shoot();
    void setUseJoystick(bool state) { useJoystick = state; }
    void launchUltimate();
    //void setWeapon(WeaponType type) { currentWeapon = type; }
   // WeaponType getWeapon() const { return currentWeapon; }

public slots:
    void update();

signals:
    void healthChanged(int health, int maxHealth);
    void died();
    void bulletFired(Bullet *bullet);
    void requestUltimate();

private:
    QPixmap sprite;
    bool wPressed, aPressed, sPressed, dPressed,fPressed;
    qreal angle;
    qreal speed;
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
   // WeaponType currentWeapon = Normal;
    double joyX = 0;
    double joyY = 0;
    bool isFiring = false;
    bool isUltiPressed = false;

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);
};

#endif // PLAYER_H
