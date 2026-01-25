#ifndef GAME_H
#define GAME_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <QList>
#include <QMouseEvent>
#include "player.h"
#include "bullet.h"
#include "spaceobject.h"
#include "enemy.h"
#include "collisionmanager.h"
#include "hud.h"
#include "levelsystem.h"
#include "xporb.h"

class Game : public QGraphicsView
{
    Q_OBJECT
public:
    Game(QWidget *parent = nullptr);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

public slots:
    void updateGame();
    void onEnemyDestroyed(Enemy *enemy);
    void onPlayerHit(Enemy *enemy);
    void onEnemyShoot(QPointF position, qreal angle);
    void onPlayerDied();
    void onLevelUp(int level);
    void onXPOrbCollected(XPOrb *orb);

private:
    QGraphicsScene *scene;
    Player *player;
    QTimer *timer;
    QList<Bullet*> bullets;
    QList<SpaceObject*> spaceObjects;
    QList<Enemy*> enemies;
    QList<XPOrb*> xpOrbs;
    CollisionManager *collisionManager;
    HUD *hud;
    LevelSystem *levelSystem;
    QPointF cameraTarget;
    qreal cameraSmoothing;
    int spawnTimer;
    int enemySpawnTimer;

    void spawnSpaceObject();
    void spawnEnemy();
};

#endif // GAME_H
