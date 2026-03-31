#include "player.h"
#include <QKeyEvent>
#include <QGraphicsScene>
#include <QtMath>
#include <QTimer>

Player::Player(QGraphicsItem *parent) : QGraphicsRectItem(parent)
{
    setRect(-30, -30, 60, 60);

    sprite = QPixmap("../../resources/spaceship.png"); // use your resource path
    if (sprite.isNull()) {
        qDebug() << "Failed to load spaceship.png";
    }

    QTransform t;
    t.rotate(90);
    sprite = sprite.transformed(t, Qt::SmoothTransformation);
    sprite = sprite.scaled(rect().size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);


    wPressed = aPressed = sPressed = dPressed = false;
    angle = 0;
    speed = 5.0;
    health = 10;
    maxHealth = 10;
    knockbackVelocity = QPointF(0, 0);
    invincibilityFrames = 0;
}

void Player::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_W) wPressed = true;
    if (event->key() == Qt::Key_A) aPressed = true;
    if (event->key() == Qt::Key_S) sPressed = true;
    if (event->key() == Qt::Key_D) dPressed = true;
    if (event->key() == Qt::Key_F) fPressed = true;
}

void Player::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_W) wPressed = false;
    if (event->key() == Qt::Key_A) aPressed = false;
    if (event->key() == Qt::Key_S) sPressed = false;
    if (event->key() == Qt::Key_D) dPressed = false;
    if (event->key() == Qt::Key_F) fPressed = false;

}

void Player::updateRotation(QPointF mousePos)
{
    QPointF centerPos = pos();
    qreal dx = mousePos.x() - centerPos.x();
    qreal dy = mousePos.y() - centerPos.y();

    angle = qAtan2(dy, dx) * 180 / M_PI;
    setRotation(angle);
}

void Player::updateMovement()
{
    qreal dx = 0;
    qreal dy = 0;

    if (wPressed) dy -= speed;
    if (sPressed) dy += speed;
    if (aPressed) dx -= speed;
    if (dPressed) dx += speed;

    // Add knockback
    dx += knockbackVelocity.x();
    dy += knockbackVelocity.y();

    // Reduce knockback over time (friction)
    knockbackVelocity *= 0.9;

    setPos(x() + dx, y() + dy);

    // Wrap around map edges
    QPointF currentPos = pos();
    qreal mapWidth = 2000;
    qreal mapHeight = 2000;
    qreal halfWidth = mapWidth / 2;
    qreal halfHeight = mapHeight / 2;

    if (currentPos.x() > halfWidth)
        setPos(-halfWidth, currentPos.y());
    else if (currentPos.x() < -halfWidth)
        setPos(halfWidth, currentPos.y());

    if (currentPos.y() > halfHeight)
        setPos(currentPos.x(), -halfHeight);
    else if (currentPos.y() < -halfHeight)
        setPos(currentPos.x(), halfHeight);
}

void Player::takeDamage(int damage)
{
    // Only take damage if not invincible
    if (invincibilityFrames > 0)
        return;

    health -= damage;
    emit healthChanged(health, maxHealth);

    // Set invincibility frames (1/3 second at 60fps)
    invincibilityFrames = 20;

    if (health <= 0)
    {
        health = 0;
        emit died();
    }
    else
    {
        // Flash red when damaged
        setBrush(Qt::red);
        QTimer::singleShot(100, this, [this]() {
            setBrush(Qt::white);
        });
    }
}

void Player::updateInvincibility()
{
    if (invincibilityFrames > 0)
    {
        invincibilityFrames--;

        // Flash white/transparent during invincibility
        if (invincibilityFrames % 10 < 5)
            setOpacity(0.5);
        else
            setOpacity(1.0);
    }
    else
    {
        setOpacity(1.0);
    }
}

void Player::pushBack(QPointF direction, qreal force)
{
    // Normalize direction and apply force
    qreal length = qSqrt(direction.x() * direction.x() + direction.y() * direction.y());
    if (length > 0)
    {
        knockbackVelocity = QPointF(
            (direction.x() / length) * force,
            (direction.y() / length) * force
            );
    }
}

void Player::update()
{
    updateMovement();
    updateInvincibility();
    QGraphicsRectItem::update();
}

void Player::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // Draw the rectangle outline (optional)
    //painter->setPen(QPen(QColor(200, 200, 255), 2));
    //painter->setBrush(Qt::NoBrush);
    //painter->drawRect(rect());

    // Draw the sprite centered inside the rectangle
    painter->drawPixmap(rect().topLeft(), sprite);
}
