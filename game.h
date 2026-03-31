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
#include "Boss.h"
#include "ultimate.h"
#include "arduinomanager.h"
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
    void onBossDestroyed(Boss *boss);
    void onEnemyShoot(QPointF position, qreal angle,bool boss);
    void onPlayerDied();
    void onLevelUp(int level);
    void onXPOrbCollected(XPOrb *orb);
    void onBossUltimate(QPointF position, qreal angle, bool isBoss);

private:
    QGraphicsScene *scene;
    Player *player;
    QTimer *timer;
    QList<Bullet*> bullets;
    QList<SpaceObject*> spaceObjects;
    QList<Enemy*> enemies;
    QList<Boss*> bosses;
    QList<XPOrb*> xpOrbs;
    CollisionManager *collisionManager;
    QList<Ultimate*> ultimates;
    HUD *hud;
    LevelSystem *levelSystem;
    QPointF cameraTarget;
    qreal cameraSmoothing;
    int spawnTimer;
    int enemySpawnTimer;
    int lastBossLevel = 0;
    int bossSpawnTimer;
    ArduinoManager *arduino;

    void spawnSpaceObject();
    void spawnEnemy();
    void spawnBoss();
    void triggerScreenClear();
};

#endif // GAME_H
