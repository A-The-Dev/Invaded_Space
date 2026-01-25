#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsRectItem>
#include <QObject>
#include <QPointF>
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>

class Player : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    Player(QGraphicsItem *parent = nullptr);

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    void updateMovement();
    void updateRotation(QPointF mousePos);
    qreal getAngle() const { return angle; }

    int getHealth() const { return health; }
    int getMaxHealth() const { return maxHealth; }
    void takeDamage(int damage);
    void pushBack(QPointF direction, qreal force);
    bool isInvincible() const { return invincibilityFrames > 0; }
    void updateInvincibility();

public slots:
    void update();

signals:
    void healthChanged(int health, int maxHealth);
    void died();

private:
    bool wPressed;
    bool aPressed;
    bool sPressed;
    bool dPressed;
    qreal angle;
    qreal speed;
    int health;
    int maxHealth;
    QPointF knockbackVelocity;
    int invincibilityFrames;
};

#endif // PLAYER_H
