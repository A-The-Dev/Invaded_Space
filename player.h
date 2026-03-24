#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsRectItem>
#include <QObject>
#include <QPointF>
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include "bullet.h"
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
    int getAttackDamage() const { return attackDamage; }
    void setAttackDamage(int val) { attackDamage = val; }
    void updateFromJoystick(double angle, double vitesse, bool tir);
    void shoot();
    //void setWeapon(WeaponType type) { currentWeapon = type; }
   // WeaponType getWeapon() const { return currentWeapon; }

public slots:
    void update();

signals:
    void healthChanged(int health, int maxHealth);
    void died();
    void bulletFired(Bullet *bullet);

private:
    QPixmap sprite;
    bool wPressed, aPressed, sPressed, dPressed;
    qreal angle;
    qreal speed;
    int health;
    int maxHealth;
    QPointF knockbackVelocity;
    int invincibilityFrames;
    int attackDamage = 1;
   // WeaponType currentWeapon = Normal;

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);
};

#endif // PLAYER_H
