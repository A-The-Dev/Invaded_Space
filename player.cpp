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
#include "SoundManager.h"

namespace AngleUtils {
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

    sprite = QPixmap("./resources/spaceship.png");
    if (sprite.isNull()) {
        qDebug() << "Failed to load spaceship.png";
    }
    grenadeShotTimer.start();
    QTransform t;
    t.rotate(90);
    sprite = sprite.transformed(t, Qt::SmoothTransformation);
    sprite = sprite.scaled(rect().size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);


    wPressed = aPressed = sPressed = dPressed = false;
    angle = 0;
    desiredAngle = 0;
    rotationSpeedDefault = 8.0;
    aimRotationSpeed = 60.0;
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
    grenadeRefillTimer = new QTimer(this);
    connect(grenadeRefillTimer, &QTimer::timeout, this, &Player::refillGrenade);
	grenadeRefillTimer->start(10000);
    numberofgrenades = maxGrenades = 3;
 
}

void Player::setEnemyLists(const QList<Enemy*> *enemiesList, const QList<Boss*> *bossesList)
{
    gameEnemies = enemiesList;
    gameBosses  = bossesList;
}

void Player::keyPressEvent(QKeyEvent *event)
{
    //qDebug() << "CLAVIER ACTIF";
    if (event->key() == Qt::Key_W) wPressed = true;
    if (event->key() == Qt::Key_A) aPressed = true;
    if (event->key() == Qt::Key_S) sPressed = true;
    if (event->key() == Qt::Key_D) dPressed = true;
    if (event->key() == Qt::Key_F) fPressed = true;

    if (event->key() == Qt::Key_G) {
        toggleTargetMode();
        qDebug() << "Target mode toggled. targetByHP =" << isTargetByHP();
    }
}

void Player::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_W) wPressed = false;
    if (event->key() == Qt::Key_A) aPressed = false;
    if (event->key() == Qt::Key_S) sPressed = false;
    if (event->key() == Qt::Key_D) dPressed = false;
    if (event->key() == Qt::Key_F) fPressed = false;

}

void Player::updateRotation(QPointF)
{
}

void Player::refillGrenade() 
{
    if (numberofgrenades < maxGrenades) 
    {
        numberofgrenades++;
        emit grenadeCountChanged(numberofgrenades);
    }
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

    qreal len = qSqrt(inputX * inputX + inputY * inputY);
    qreal moveX = 0;
    qreal moveY = 0;
    if (len > 0.0) {
        moveX = (inputX / len) * speed;
        moveY = (inputY / len) * speed;
    }

    moveX += knockbackVelocity.x();
    moveY += knockbackVelocity.y();

    knockbackVelocity *= 0.9;

    setPos(x() + moveX, y() + moveY);

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
void Player::setGrenadeCount(int newgrenadecount) 
{
    numberofgrenades = newgrenadecount;
    emit grenadeCountChanged(numberofgrenades);
}
void Player::takeDamage(int damage)
{
    if (invincibilityFrames > 0)
        return;

    health -= damage;
    emit healthChanged(health, maxHealth);

    SoundManager::instance()->playSound(SoundManager::PlayerHurt);

    invincibilityFrames = 20;

    if (health <= 0)
    {
        health = 0;

        SoundManager::instance()->playSound(SoundManager::PlayerDeath);

        emit died();
    }
    else
    {
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
        
        SoundManager::instance()->playSound(SoundManager::PlayerUltimate);
        
        return true;
    }
    return false;
}

void Player::pushBack(QPointF direction, qreal force)
{
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
    health += amount;
    emit healthChanged(health, maxHealth);
}

void Player::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (sprite.isNull()) {
        painter->setPen(QPen(QColor(200, 200, 255), 2));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(rect());
        return;
    }

    if (!shipColor.isValid() || shipColor.alpha() == 0)
    {
        painter->drawPixmap(rect().topLeft(), sprite);
        return;
    }

    QPixmap coloredSprite(sprite.size());
    coloredSprite.fill(Qt::transparent);

    QPainter spritePainter(&coloredSprite);

    spritePainter.drawPixmap(0, 0, sprite);

    spritePainter.setCompositionMode(QPainter::CompositionMode_SourceAtop);

    QColor reducedColor = shipColor;
    reducedColor.setAlpha(static_cast<int>(shipColor.alpha() * 0.8));
    
    spritePainter.fillRect(coloredSprite.rect(), reducedColor);
    
    spritePainter.end();

    painter->drawPixmap(rect().topLeft(), coloredSprite);
}

void Player::updateFromJoystick(double axisX, double axisY, bool tir, bool ulti, bool grenade)
{
    this->joyX = axisX;
    this->joyY = axisY;
    this->isFiring = tir;
	this->isUltiPressed = ulti;
    this->isGrenadePressed = grenade;
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
        qreal d2 = dx * dx + dy * dy;

        if (targetByHP) {
            if (hp > bestHP || (qFuzzyCompare(hp, bestHP) && d2 < bestDist2)) {
                bestHP = hp;
                bestDist2 = d2;
                chosenPos = p;
                found = true;
            }
        }
        else {
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
    }
    else {
        QGraphicsScene* sc = scene();
        if (sc) {
            const QList<QGraphicsItem*> items = sc->items();
            for (QGraphicsItem* it : items) {
                Enemy* e = dynamic_cast<Enemy*>(it);
                if (e) {
                    considerTarget(e->pos(), static_cast<qreal>(e->getHealth()));
                    continue;
                }
                Boss* b = dynamic_cast<Boss*>(it);
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

    Bullet* bullet = new Bullet(this->pos(), targetAngle);
    bullet->setDamage(attackDamage);

    emit bulletFired(bullet);
    
    SoundManager::instance()->playSound(SoundManager::PlayerShoot);

    lastShotTimer.restart();
}
void Player::throwGrenade() 
{
    if (grenadeShotTimer.elapsed() < msBetweenGrenades) return;

    if (numberofgrenades <= 0) return;
    qreal targetAngle = desiredAngle;
    QPointF myPos = pos();

    bool found = false;
    qreal bestDist2 = std::numeric_limits<qreal>::infinity();
    qreal bestHP = -std::numeric_limits<qreal>::infinity();
    QPointF chosenPos;

    auto considerTarget = [&](QPointF p, qreal hp) {
        qreal dx = p.x() - myPos.x();
        qreal dy = p.y() - myPos.y();
        qreal d2 = dx * dx + dy * dy;

        if (targetByHP) {
            if (hp > bestHP || (qFuzzyCompare(hp, bestHP) && d2 < bestDist2)) {
                bestHP = hp;
                bestDist2 = d2;
                chosenPos = p;
                found = true;
            }
        }
        else {
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
    }
    else {
        QGraphicsScene* sc = scene();
        if (sc) {
            const QList<QGraphicsItem*> items = sc->items();
            for (QGraphicsItem* it : items) {
                Enemy* e = dynamic_cast<Enemy*>(it);
                if (e) {
                    considerTarget(e->pos(), static_cast<qreal>(e->getHealth()));
                    continue;
                }
                Boss* b = dynamic_cast<Boss*>(it);
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
    Grenades* grenade = new Grenades(this->pos(), targetAngle);
    activeGrenades.append(grenade);
    emit grenadeThrown(grenade);
    numberofgrenades--;
    emit grenadeCountChanged(numberofgrenades);
    grenadeShotTimer.restart();

    SoundManager::instance()->playSound(SoundManager::GrenadeThrow);
}
void Player::launchUltimate() {
    QPointF spawnPos = this->scenePos();

    Ultimate* myUltimate = new Ultimate(spawnPos, this->rotation(), true, this, nullptr, false);

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
    if (qAbs(joyX) > 0.1 || qAbs(joyY) > 0.1) {
        this->setPos(x() + (joyX * this->speed), y() + (joyY * this->speed));

        qreal targetAngle = qAtan2(joyY, joyX) * 180 / M_PI;
        this->setRotation(targetAngle);
    }

    if (isFiring) {
        if (lastShotTimer.elapsed() > 100) {
            this->shoot();
            lastShotTimer.restart();
        }
    }

    if (isUltiPressed && isUltimateReady) {
        this->isUltimateReady = false;
        emit requestUltimate();
        }
    if (isGrenadePressed) {
        this->throwGrenade();
	}
}
void Player::removeActiveGrenade(int index) 
{
    if (index >= 0 && index < activeGrenades.size()) 
    {
        activeGrenades.removeAt(index);
    }
}

void Player::resetInputStates()
{
    wPressed = aPressed = sPressed = dPressed = fPressed = false;
    useJoystick = false;
    knockbackVelocity = QPointF(0,0);
    aimHoldFrames = 0;
    rotationSpeedCurrent = rotationSpeedDefault;
}

void Player::resetToDefault()
{
    health = maxHealth = 20;
    speed = 5.0;
    invincibilityFrames = 0;
    
    ultimateCharge = 0.0f;
    isUltimateReady = false;
    attackDamage = 1;

    numberofgrenades = 3;
    for (Grenades* g : activeGrenades) {
        if (g->scene()) scene()->removeItem(g);
        delete g;
    }
    activeGrenades.clear();
    
    // Reset movement state
    resetInputStates();
    
    // Emit signal for UI update
    emit healthChanged(health, maxHealth);
    emit grenadeCountChanged(numberofgrenades);
}
