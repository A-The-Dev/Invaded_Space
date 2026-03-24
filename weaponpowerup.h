#ifndef WEAPONPOWERUP_H
#define WEAPONPOWERUP_H

#include <QGraphicsRectItem>
#include <QObject>

enum WeaponType { Normal, Spread, RapidFire };

class WeaponPowerUp : public QObject, public QGraphicsRectItem {
    Q_OBJECT
public:
    WeaponPowerUp(QPointF pos, WeaponType type, QGraphicsItem *parent = nullptr);
    WeaponType getType() const { return type; }
    void moveTowardsPlayer(QPointF playerPos);

private:
    WeaponType type;
    qreal attractionSpeed = 0;
};
#endif // WEAPONPOWERUP_H
