#include "player.h"
#include <QKeyEvent>
#include <QGraphicsScene>
#include <QtMath>
#include <QTimer>
#include <QDebug>
#include "bullet.h"
#include "enemy.h"
#include "Boss.h"
#include <limits>
#include "ultimate.h"

namespace AngleUtils {
    // normalize difference to [-180,180)
    static qreal AngleDelta(qreal fromDeg, qreal toDeg) {
        qreal diff = toDeg - fromDeg;
        while (diff < -180.0) diff += 360.0;
        while (diff >= 180.0) diff -= 360.0;
        return diff;
    }
}

Player::Player(QGraphicsItem *parent) : QGraphicsRectItem(parent)
{
    setRect(-30, -30, 60, 60);

    sprite = QPixmap("./resources/spaceship.png"); // use your resource path
    if (sprite.isNull()) {
        qDebug() << "Failed to load spaceship.png";
    }

    QTransform t;
    t.rotate(90);
    sprite = sprite.transformed(t, Qt::SmoothTransformation);
    sprite = sprite.scaled(rect().size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);


    wPressed = aPressed = sPressed = dPressed = false;
    angle = 0;
    desiredAngle = 0;
    rotationSpeedDefault = 8.0; // degrees per frame; lower = slower/smoother
    aimRotationSpeed = 60.0;    // when aiming at target rotate much faster
    rotationSpeedCurrent = rotationSpeedDefault;
    speed = 5.0;
    maxSpeed = 20.0;
    health = 20;
    maxHealth = 20;
    knockbackVelocity = QPointF(0, 0);
    invincibilityFrames = 0;
    lastShotTimer.start();
    attackDamage = 1;
    aimHoldFrames = 0;
}

void Player::setEnemyLists(const QList<Enemy*> *enemiesList, const QList<Boss*> *bossesList)
{
    gameEnemies = enemiesList;
    gameBosses  = bossesList;
}

void Player::keyPressEvent(QKeyEvent *event)
{
    if (useJoystick){
        //qDebug() << "BLOQUÉ !";
        return;
    }
    //qDebug() << "CLAVIER ACTIF";
    if (event->key() == Qt::Key_W) wPressed = true;
    if (event->key() == Qt::Key_A) aPressed = true;
    if (event->key() == Qt::Key_S) sPressed = true;
    if (event->key() == Qt::Key_D) dPressed = true;
    if (event->key() == Qt::Key_F) fPressed = true;

    // Toggle targeting mode on G press
    if (event->key() == Qt::Key_G) {
        toggleTargetMode();
        qDebug() << "Target mode toggled. targetByHP =" << isTargetByHP();
    }
}

void Player::keyReleaseEvent(QKeyEvent *event)
{
    if (useJoystick) {
        return;
    }
    if (event->key() == Qt::Key_W) wPressed = false;
    if (event->key() == Qt::Key_A) aPressed = false;
    if (event->key() == Qt::Key_S) sPressed = false;
    if (event->key() == Qt::Key_D) dPressed = false;
    if (event->key() == Qt::Key_F) fPressed = false;

}

// Mouse-based aiming disabled: always auto-aim now.
// Keep the function a no-op so Game::mouseMoveEvent can remain unchanged
void Player::updateRotation(QPointF /*mousePos*/)
{
    // intentionally empty to prevent mouse from changing ship rotation
}

void Player::setSpeed(qreal s)
{
    qreal newSpeed = s;
    if (newSpeed < 0.1) newSpeed = 0.1;
    if (newSpeed > maxSpeed) newSpeed = maxSpeed;
    speed = newSpeed;
}

void Player::updateMovement()
{
    qreal inputX = 0;
    qreal inputY = 0;

    if (wPressed) inputY -= 1.0;
    if (sPressed) inputY += 1.0;
    if (aPressed) inputX -= 1.0;
    if (dPressed) inputX += 1.0;

    // Normalize input vector so diagonal movement isn't faster
    qreal len = qSqrt(inputX * inputX + inputY * inputY);
    qreal moveX = 0;
    qreal moveY = 0;
    if (len > 0.0) {
        moveX = (inputX / len) * speed;
        moveY = (inputY / len) * speed;
    }

    // Add knockback (kept as-is)
    moveX += knockbackVelocity.x();
    moveY += knockbackVelocity.y();

    // Friction on knockback
    knockbackVelocity *= 0.9;

    setPos(x() + moveX, y() + moveY);

    // Determine desiredAngle:
    qreal moveLen2 = moveX*moveX + moveY*moveY;
    const qreal moveThreshold2 = 0.01;

    if (aimHoldFrames > 0) {
        --aimHoldFrames;
        rotationSpeedCurrent = aimRotationSpeed;
    } else {
        rotationSpeedCurrent = rotationSpeedDefault;
        if (moveLen2 > moveThreshold2) {
            desiredAngle = qAtan2(moveY, moveX) * 180.0 / M_PI;
        }
    }

    qreal diff = AngleUtils::AngleDelta(angle, desiredAngle);
    qreal step = rotationSpeedCurrent;
    if (diff > step) diff = step;
    else if (diff < -step) diff = -step;
    angle += diff;
    while (angle < -180.0) angle += 360.0;
    while (angle >= 180.0) angle -= 360.0;
    setRotation(angle);

    // Wrap
    QPointF currentPos = pos();
    qreal mapWidth = 2000;
    qreal mapHeight = 2000;
    qreal halfWidth = mapWidth / 2;
    qreal halfHeight = mapHeight / 2;

    if (currentPos.x() > halfWidth) setPos(-halfWidth, currentPos.y());
    else if (currentPos.x() < -halfWidth) setPos(halfWidth, currentPos.y());

    if (currentPos.y() > halfHeight) setPos(currentPos.x(), -halfHeight);
    else if (currentPos.y() < -halfHeight) setPos(currentPos.x(), halfHeight);
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

void Player::gainUltimateCharge(float amount)
{
    ultimateCharge = qMin(maxUltimateCharge, ultimateCharge + amount);
    if (ultimateCharge >= maxUltimateCharge) isUltimateReady = true;
}

bool Player::tryUseUltimate()
{
    if (isUltimateReady) {
        ultimateCharge = 0;
        isUltimateReady = false;
        return true;
    }
    return false;
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
void Player::refillHealth() {
    health = maxHealth;
    emit healthChanged(health, maxHealth);
}

void Player::increaseMaxHealth(int amount) {
    maxHealth += amount;
    health += amount; // On donne aussi un petit bonus de vie actuelle
    emit healthChanged(health, maxHealth);
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

void Player::updateFromJoystick(double axisX, double axisY, bool tir, bool ulti)
{
    this->joyX = axisX;
    this->joyY = axisY;
    this->isFiring = tir;
	this->isUltiPressed = ulti;
}

void Player::shoot() {
    // On utilise pos() pour la position et l'angle actuel du vaisseau
    Bullet *bullet = new Bullet(this->pos(), this->rotation());

    if (len > 0.1) {
        desiredAngle = qAtan2(ay, ax) * 180.0 / M_PI;
    }
    emit bulletFired(bullet);
}
void Player::launchUltimate() {
    qDebug() << "Tentative d'apparition de l'ultime...";

    if (tir) {
        if (lastShotTimer.elapsed() > msBetweenShots) {
            this->shoot();
            lastShotTimer.restart();
        }
    }
}
    //  Calculer la position 
    QPointF spawnPos = this->scenePos();

    //  Créer l'objet
    Ultimate* myUltimate = new Ultimate(spawnPos, this->rotation(), true, this, nullptr, false);

    // Vérifier la scène 
    QGraphicsScene* currentScene = this->scene();
    if (currentScene) {
        currentScene->addItem(myUltimate);
        myUltimate->setZValue(101);
        qDebug() << "Succès : Ultime ajouté à la scène à : " << spawnPos;
    }
    else {
        qDebug() << "ERREUR : Impossible de trouver la scène !";
    }

    this->isUltimateReady = false;
}
void Player::processMovement()
{
    qreal speed = 8.0; 

    // Mouvement basé sur les dernières valeurs reçues
    if (qAbs(joyX) > 0.1 || qAbs(joyY) > 0.1) {
        this->setPos(x() + (joyX * speed), y() + (joyY * speed));

        qreal targetAngle = qAtan2(joyY, joyX) * 180 / M_PI;
        this->setRotation(targetAngle);
    }

void Player::shoot() {
    if (lastShotTimer.elapsed() < msBetweenShots) return;

    qreal targetAngle = desiredAngle;
    QPointF myPos = pos();

    bool found = false;
    qreal bestDist2 = std::numeric_limits<qreal>::infinity();
    qreal bestHP = -std::numeric_limits<qreal>::infinity();
    QPointF chosenPos;

    auto considerTarget = [&](QPointF p, qreal hp) {
        qreal dx = p.x() - myPos.x();
        qreal dy = p.y() - myPos.y();
        qreal d2 = dx*dx + dy*dy;

        if (targetByHP) {
            if (hp > bestHP || (qFuzzyCompare(hp, bestHP) && d2 < bestDist2)) {
                bestHP = hp;
                bestDist2 = d2;
                chosenPos = p;
                found = true;
            }
        } else {
            if (d2 < bestDist2) {
                bestDist2 = d2;
                chosenPos = p;
                found = true;
            }
        }
    };

    if (gameEnemies || gameBosses) {
        if (gameEnemies) {
            for (Enemy* e : *gameEnemies) {
                if (!e) continue;
                considerTarget(e->pos(), static_cast<qreal>(e->getHealth()));
            }
        }
        if (gameBosses) {
            for (Boss* b : *gameBosses) {
                if (!b) continue;
                considerTarget(b->pos(), static_cast<qreal>(b->getHealth()));
            }
        }
    } else {
        QGraphicsScene *sc = scene();
        if (sc) {
            const QList<QGraphicsItem*> items = sc->items();
            for (QGraphicsItem *it : items) {
                Enemy *e = dynamic_cast<Enemy*>(it);
                if (e) {
                    considerTarget(e->pos(), static_cast<qreal>(e->getHealth()));
                    continue;
                }
                Boss *b = dynamic_cast<Boss*>(it);
                if (b) {
                    considerTarget(b->pos(), static_cast<qreal>(b->getHealth()));
                }
            }
        }
    }

    if (found) {
        qreal dx = chosenPos.x() - myPos.x();
        qreal dy = chosenPos.y() - myPos.y();
        targetAngle = qAtan2(dy, dx) * 180.0 / M_PI;

        desiredAngle = targetAngle;
        aimHoldFrames = 12;
        rotationSpeedCurrent = aimRotationSpeed;
    }

    Bullet *bullet = new Bullet(this->pos(), targetAngle);
    bullet->setDamage(attackDamage);
    // Tir 
    if (isFiring) {
        if (lastShotTimer.elapsed() > 100) {
            this->shoot();
            lastShotTimer.restart();
        }
    }
    if (isUltiPressed && isUltimateReady) {
        this->isUltimateReady = false;
        emit requestUltimate();
        //this->launchUltimate();

    emit bulletFired(bullet);

    lastShotTimer.restart();
}

// resetInputStates implementation
void Player::resetInputStates()
{
    wPressed = aPressed = sPressed = dPressed = fPressed = false;
    useJoystick = false;
    knockbackVelocity = QPointF(0,0);
    aimHoldFrames = 0;
    rotationSpeedCurrent = rotationSpeedDefault;
}
