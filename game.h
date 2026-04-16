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
#include "JsonParser.h"
#include "leaderboard.h"
#include <QGraphicsProxyWidget>

class Menu;

class Game : public QGraphicsView
{
    Q_OBJECT
public:
    // If startPaused is true the game loop / spawns won't start until startGame() is called.
    explicit Game(QWidget *parent = nullptr, bool startPaused = false);

    // Start the game loop and initial spawns (call when menu Start pressed).
    void startGame();
    
    // Access to player for customization
    Player* getPlayer() { return player; }

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

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
    void toggleFullscreen();
    void onPauseMenuRequested();
    void onResumeGame();

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
    int currentScore = 0;
    bool UltUsed = false;
    bool bossSpawnRequested = false;

    bool isFullscreen = false;
    QRect previousGeometry;
    int currentBossID = 0;
    bool m_isPaused = false;

    bool m_gameStarted = false;
    bool m_upgradeMenuOpen = false;

    Menu *m_pauseMenu;

    void spawnSpaceObject();
    void spawnEnemy();
    void spawnBoss();
    void triggerScreenClear();
};

#endif // GAME_H
