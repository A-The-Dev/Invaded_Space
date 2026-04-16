
#include "game.h"
#include "upgrade_menu.h"
#include "menu.h"
#include "SoundManager.h"
#include <QKeyEvent>
#include <QLineF>
#include <QRandomGenerator>
#include <QtMath>
#include <QDebug>
#include "arduinomanager.h"
#include <QFocusEvent>
#include <QResizeEvent>

Game::Game(QWidget *parent, bool startPaused) : QGraphicsView(parent),
    scene(nullptr), player(nullptr), timer(nullptr),
    collisionManager(nullptr), hud(nullptr), levelSystem(nullptr),
    arduino(nullptr), m_gameStarted(false), m_pauseMenu(nullptr)
{
    scene = new QGraphicsScene(this);
    scene->setSceneRect(-1000, -1000, 2000, 2000);
    hud = new HUD(scene, this);
    scene->setBackgroundBrush(QBrush(QColor(10, 10, 30)));

    setScene(scene);

    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setCacheMode(QGraphicsView::CacheBackground);
    setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing, true);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    resize(800, 600);

    setMouseTracking(true);

    player = new Player();
    player->setPos(0, 0);
    scene->addItem(player);

    player->setEnemyLists(&enemies, &bosses);

    arduino = new ArduinoManager(this);
    if (arduino->connectToArduino("COM3")) {
        qDebug() << "Connexion Arduino REUSSIE sur COM3";
        player->setUseJoystick(true);
    } else {
        qDebug() << "Connexion Arduino ECHOUEE. Mode clavier actif.";
        player->setUseJoystick(false);
    }

    connect(arduino, &ArduinoManager::commandReceived, this, [this](double x, double y, bool tir, bool ulti, bool grenade, bool BossSpawn, bool pause, bool volume) {
        player->updateFromJoystick(x, y, tir, ulti, grenade);
        this->bossSpawnRequested = BossSpawn;
        if (pause && !m_isPaused) {
            // On appelle le menu de pause 
            this->onPauseMenuRequested();
        }
        SoundManager::instance()->setVolume(volume);
    });


    connect(player, &Player::grenadeCountChanged, hud, &HUD::updateGrenades);

    hud->setMaxGrenades(player->getMaxGrenades());
    hud->updateGrenades(player->getGrenadeCount());


    connect(player, &Player::bulletFired, this, [this](Bullet* b){
        scene->addItem(b);
        bullets.append(b);
    });
    connect(player, &Player::requestUltimate, this, &Game::triggerScreenClear);

	connect(player, &Player::grenadeThrown, this, [this](Grenades* g) 
        {
            scene->addItem(g);
        });
    cameraTarget = player->pos();
    cameraSmoothing = 0.1;
    centerOn(player);

    spawnTimer = 0;
    enemySpawnTimer = 0;
    bossSpawnTimer = 0;

    collisionManager = new CollisionManager(this);
    connect(collisionManager, &CollisionManager::enemyDestroyed, this, &Game::onEnemyDestroyed);
    connect(collisionManager, &CollisionManager::playerHitEnemy, this, &Game::onPlayerHit);
    connect(collisionManager, &CollisionManager::playerHitBoss, this, &Game::onPlayerHit);



    connect(player, &Player::died, this, &Game::onPlayerDied);
    connect(player, &Player::healthChanged, this, [this](int health, int maxHealth) {
        if (hud) hud->updateHealth(health, maxHealth);
    });

    hud->updateHealth(player->getHealth(), player->getMaxHealth());

    levelSystem = new LevelSystem(this);
    connect(levelSystem, &LevelSystem::levelUp, this, &Game::onLevelUp);
    connect(levelSystem, &LevelSystem::xpChanged, hud, &HUD::updateXP);
    hud->updateLevel(levelSystem->getLevel());
    hud->updateXP(levelSystem->getCurrentXP(), levelSystem->getXPForNextLevel());

    m_pauseMenu = new Menu(this);
    m_pauseMenu->hide();
    connect(m_pauseMenu, &Menu::resumeGameRequested, this, &Game::onResumeGame);
    connect(m_pauseMenu, &Menu::fullscreenToggled, this, &Game::toggleFullscreen);
    connect(m_pauseMenu, &Menu::volumeChanged, this, [](int value) { Q_UNUSED(value);});

    if (!startPaused) {
        startGame();
    }
}

void Game::startGame()
{
    if (m_gameStarted) return;
    m_gameStarted = true;

    setProperty("gameStarted", true);

    for (int i = 0; i < 50; ++i)
    {
        spawnSpaceObject();
    }

    for (int i = 0; i < 6; ++i)
    {
        spawnEnemy();
    }

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Game::updateGame);
    timer->start(16);

    setWindowTitle("Invaded Space - WASD to move, Click to shoot");
}


void Game::spawnSpaceObject()
{
    QRandomGenerator *rng = QRandomGenerator::global();

    qreal x = rng->bounded(-1000, 1000);
    qreal y = rng->bounded(-1000, 1000);

    int typeRoll = rng->bounded(100);
    SpaceObject::ObjectType type;

    if (typeRoll < 70)
        type = SpaceObject::Star;
    else if (typeRoll < 85)
        type = SpaceObject::Planet;
    else
        type = SpaceObject::Asteroid;

    SpaceObject *obj = new SpaceObject(type);
    obj->setPos(x, y);
    scene->addItem(obj);
    spaceObjects.append(obj);
}

void Game::spawnEnemy()
{
    QRandomGenerator *rng = QRandomGenerator::global();

    // Spawn away from player
    QPointF playerPos = player->pos();
    qreal angle = rng->bounded(360) * M_PI / 180;
    qreal distance = rng->bounded(400, 700);

    QPointF spawnPos(
        playerPos.x() + qCos(angle) * distance,
        playerPos.y() + qSin(angle) * distance
        );

    int typeRoll = rng->bounded(100);
    Enemy::EnemyType type;

    if (typeRoll < 40)
        type = Enemy::Chaser;
    else if (typeRoll < 65)
        type = Enemy::Wanderer;
    else if (typeRoll < 80)
        type = Enemy::Stationary;
    else
        type = Enemy::Shooter;

    Enemy *enemy = new Enemy(type, spawnPos);
    connect(enemy, &Enemy::destroyed, this, [this, enemy]() {
        onEnemyDestroyed(enemy);
    });
    connect(enemy, &Enemy::shootBullet, this, &Game::onEnemyShoot);
    scene->addItem(enemy);
    enemies.append(enemy);
}

void Game::spawnBoss()
{
    QRandomGenerator *rng = QRandomGenerator::global();

    // Spawn away from player
    QPointF playerPos = player->pos();
    qreal angle = rng->bounded(360) * M_PI / 180;
    qreal distance = rng->bounded(400, 700);

    QPointF spawnPos(
        playerPos.x() + qCos(angle) * distance,
        playerPos.y() + qSin(angle) * distance
        );

    int typeRoll = rng->bounded(100);
    Boss::BossType type;

    if (typeRoll < 33) {
        type = Boss::Boss1;
        currentBossID = 1;
    } else if (typeRoll < 66) {
        type = Boss::Boss2;
        currentBossID = 2;
    } else {
        type = Boss::Boss4;
        currentBossID = 3;
    }

    Boss *boss = new Boss(type, spawnPos);

    connect(boss, &Boss::destroyed, this, [this, boss]() { onBossDestroyed(boss); });
    connect(boss, &Boss::shootBullet, this, &Game::onEnemyShoot);
    connect(boss, &Boss::UseUltimate, this, &Game::onBossUltimate);
    scene->addItem(boss);
    bosses.append(boss);
}

void Game::mouseMoveEvent(QMouseEvent *event)
{
    QPointF scenePos = mapToScene(event->pos());
    player->updateRotation(scenePos);
}

void Game::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        player->shoot();
    }
    else if(event->button() == Qt::RightButton)
    {
        player->throwGrenade();
	}
}

void Game::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        if (m_gameStarted && !m_isPaused && !m_upgradeMenuOpen && m_pauseMenu && !m_pauseMenu->isPauseMenuVisible()) {
            onPauseMenuRequested();
            event->accept();
            return;
        }
    }

    player->keyPressEvent(event);
    if (event->key() == Qt::Key_F)
    {
        if (!m_upgradeMenuOpen && player->tryUseUltimate()) {
            triggerScreenClear();
        }
    }

    if (event->key() == Qt::Key_F11)
        toggleFullscreen();
}

void Game::triggerScreenClear()
{
	UltUsed = true;
    while (!enemies.isEmpty())
    {
        Enemy* e = enemies.takeFirst();

        if (scene->views().first()->sceneRect().contains(e->pos()))
            onEnemyDestroyed(e);
        else
        {
            scene->removeItem(e);
            e->deleteLater();
        }
    }


    for (Boss* b : bosses) {
        b->takeDamage(100);
    }
    QPixmap ultImg("./resources/Léanuke.png");
    QGraphicsPixmapItem* flash = new QGraphicsPixmapItem(ultImg);

    flash->setOffset(-ultImg.width()/2, -ultImg.height()/2);
    flash->setPos(player->pos());

    flash->setZValue(2000);
    flash->setOpacity(0.8);

    scene->addItem(flash);

    QTimer::singleShot(150, [this, flash]() {
        scene->removeItem(flash);
        delete flash;
    });

	UltUsed = false;
    hud->updateUltimate(player->getUltimatePercentage());
}

void Game::keyReleaseEvent(QKeyEvent *event)
{
    player->keyReleaseEvent(event);
}

void Game::onEnemyDestroyed(Enemy *enemy)
{
    currentScore += 10;
    int xpValue = 20;
    XPOrb *orb = new XPOrb(enemy->pos(), xpValue);
    scene->addItem(orb);
    xpOrbs.append(orb);

    enemies.removeOne(enemy);
    scene->removeItem(enemy);
    enemy->deleteLater();
    if (UltUsed == true)
    {
		player->resetUltimateCharge();
    }
	else if (UltUsed == false)
    {
        player->gainUltimateCharge(5.0);
        hud->updateUltimate(player->getUltimatePercentage());
    }

}
void Game::onBossDestroyed(Boss *boss)
{
    currentScore += 100;
    int xpValue = 500;
    XPOrb *orb = new XPOrb(boss->pos(), xpValue);
    scene->addItem(orb);
    xpOrbs.append(orb);

    bosses.removeOne(boss);
    scene->removeItem(boss);
    boss->deleteLater();
    if (UltUsed == true)
    {
        player->resetUltimateCharge();
    }
    else if(UltUsed == false)
    {
        player->gainUltimateCharge(5.0);
        hud->updateUltimate(player->getUltimatePercentage());
    }
}

void Game::onPlayerHit(Enemy *enemy)
{
    // Player already gets pushed back in collision manager
}

void Game::onEnemyShoot(QPointF position, qreal angle,bool boss)
{
    if( boss == true)
    {   
        Boss::BossType type = bosses[0]->getType();
        if( type == Boss::Boss1)
        {
            Bullet *bullet = new Bullet(position, angle, false,Bullet::Boss1,true);
            scene->addItem(bullet);
            bullets.append(bullet);

        }
        else if(type == Boss::Boss2)
        {
            Bullet *bullet = new Bullet(position, angle, false,Bullet::Boss2,true);
            scene->addItem(bullet);
            bullets.append(bullet);
        }
        else if(type == Boss::Boss4)
        {
            Bullet *bullet = new Bullet(position, angle, false,Bullet::Boss4,true);
            scene->addItem(bullet);
            bullets.append(bullet);

        }
    }
    else
    {
        Bullet *bullet = new Bullet(position, angle, false);
        scene->addItem(bullet);
        bullets.append(bullet);
    }
}

void Game::onBossUltimate(QPointF position, qreal angle, bool isBoss) {
    Boss* senderBoss = qobject_cast<Boss*>(sender());



    if (senderBoss->getType() == Boss::Boss1) {
        for (int i = 0; i < 8; ++i) {
            Ultimate *u = new Ultimate(position, i * 45, false, nullptr, senderBoss, true);
            scene->addItem(u);
            ultimates.append(u);
        }
    }
    else if (senderBoss->getType() == Boss::Boss2) {
        qreal rad = (angle + 90) * M_PI / 180.0;

        for (int i = -2; i <= 2; ++i) {
            qreal spacing = i * 60;
            QPointF wallPos = position + QPointF(qCos(rad) * spacing, qSin(rad) * spacing);

            Ultimate *u = new Ultimate(wallPos, angle, false, nullptr, senderBoss, true);
            scene->addItem(u);
            ultimates.append(u);
        }
    }
    else if (senderBoss->getType() == Boss::Boss4) {
        Ultimate *laser = new Ultimate(position, angle, false, nullptr, senderBoss, true);
        scene->addItem(laser);
        ultimates.append(laser);

        QTimer::singleShot(600, this, [this, laser]() {
            if (ultimates.contains(laser)) {
                ultimates.removeOne(laser);
                scene->removeItem(laser);
                delete laser;
            }
        });
    }
}

void Game::onPlayerDied()
{
    if (timer) timer->stop();
    JsonParser::savePlayerResult(player, levelSystem->getLevel(), currentScore);

    Leaderboard* lb = new Leaderboard(this);
    lb->setEndgameMode(true);
    lb->refresh();

    int lw = qBound(400, width() * 60 / 100, width() - 40);
    int lh = qBound(300, height() * 70 / 100, height() - 40);
    int x = (width() - lw) / 2;
    int y = (height() - lh) / 2;
    
    lb->setGeometry(x, y, lw, lh);
    lb->setFocusPolicy(Qt::StrongFocus);
    lb->show();
    lb->raise();
    lb->setFocus();

    connect(lb, &Leaderboard::restartRequested, this, [this, lb]() {
        lb->deleteLater();

        m_isPaused = false;
        m_gameStarted = false;

        qDeleteAll(enemies);
        enemies.clear();
        qDeleteAll(bosses);
        bosses.clear();
        qDeleteAll(bullets);
        bullets.clear();
        qDeleteAll(xpOrbs);
        xpOrbs.clear();
        qDeleteAll(spaceObjects);
        spaceObjects.clear();
        qDeleteAll(ultimates);
        ultimates.clear();

        levelSystem->reset();
        currentScore = 0;
        lastBossLevel = 0;

        player->resetToDefault();
        player->setPos(0, 0);

        spawnTimer = 0;
        enemySpawnTimer = 0;
        bossSpawnTimer = 0;

        cameraTarget = player->pos();

        hud->updateLevel(levelSystem->getLevel());
        hud->updateHealth(player->getHealth(), player->getMaxHealth());
        hud->updateXP(levelSystem->getCurrentXP(), levelSystem->getXPForNextLevel());
        hud->updateUltimate(player->getUltimatePercentage());

        startGame();
    });

    connect(lb, &Leaderboard::quitRequested, this, [this, lb]() {
        lb->deleteLater();
        qApp->quit();
    });

    m_isPaused = true;
}

void Game::onLevelUp(int level) {
    timer->stop();

    if (player) player->resetInputStates();

    disconnect(arduino, &ArduinoManager::commandReceived, player, &Player::updateFromJoystick);

    UpgradeMenu* menu = new UpgradeMenu(this);

    m_upgradeMenuOpen = true;

    connect(arduino, &ArduinoManager::commandReceived, menu, &UpgradeMenu::navigateWithJoystick);

    connect(menu, &UpgradeMenu::upgradeSelected, [this](int choice) {
        if (choice == 0) {
            qreal increment = 0.4;
            player->setSpeed(player->getSpeed() + increment);
            qDebug() << "Applied speed upgrade, new speed =" << player->getSpeed();
        }
        else if (choice == 1) {
            player->setAttackDamage(player->getAttackDamage() + 1);
            qDebug() << "Applied damage upgrade, new damage =" << player->getAttackDamage();
        }
        else if (choice == 2) {
            player->refillHealth();
            player->increaseMaxHealth(2);
            qDebug() << "Applied health upgrade, new health =" << player->getHealth();
        }
        });

    menu->exec();

    m_upgradeMenuOpen = false;

    disconnect(arduino, &ArduinoManager::commandReceived, menu, &UpgradeMenu::navigateWithJoystick);

    connect(arduino, &ArduinoManager::commandReceived, player, &Player::updateFromJoystick);

    timer->start();

    hud->updateLevel(level);
    setWindowTitle(QString("Invaded Space - Level %1").arg(level));
}

void Game::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    if (player) player->resetInputStates();
    QGraphicsView::focusOutEvent(event);
}

void Game::onXPOrbCollected(XPOrb *orb)
{
    levelSystem->addXP(orb->getXPValue());
    xpOrbs.removeOne(orb);
    scene->removeItem(orb);
    delete orb;
}

void Game::updateGame()
{
    player->update();
    QPointF playerPos = player->pos();
    cameraTarget.setX(cameraTarget.x() + (playerPos.x() - cameraTarget.x()) * cameraSmoothing);
    cameraTarget.setY(cameraTarget.y() + (playerPos.y() - cameraTarget.y()) * cameraSmoothing);
    centerOn(cameraTarget);

    hud->updatePosition(cameraTarget, player->pos());

    QList<SpaceObject*> objectsToRemove;
    for (int i = 0; i < spaceObjects.size(); ++i)
    {
        spaceObjects[i]->update();

        if (spaceObjects[i]->property("lifetime").toReal() <= 0)
        {
            objectsToRemove.append(spaceObjects[i]);
        }
    }

    for (int i = 0; i < objectsToRemove.size(); ++i)
    {
        spaceObjects.removeOne(objectsToRemove[i]);
        scene->removeItem(objectsToRemove[i]);
        delete objectsToRemove[i];
    }

    spawnTimer++;
    if (spawnTimer >= 120)
    {
        spawnTimer = 0;
        spawnSpaceObject();
    }

    enemySpawnTimer++;
    if (enemySpawnTimer >= 60)
    {
        if (enemies.size() < 5 * levelSystem->getLevel())
        {
            spawnEnemy();
        }
    }


    if (bossSpawnRequested)
    {
        if (levelSystem->getLevel() > 15 && bosses.size() < 2)
        {
            spawnBoss();
            bossSpawnRequested = false;
        }
        else if(levelSystem->getLevel() < 15 && bosses.size() < 1)
        {
            spawnBoss();
            bossSpawnRequested = false;
		}

    }
    for (int i = ultimates.size() - 1; i >= 0; --i)
    {
        ultimates[i]->move();

        if (ultimates[i]->getSpeed() > 0 && collisionManager->isOffScreen(ultimates[i], cameraTarget, 2000, 2000)) {
            Ultimate* u = ultimates.takeAt(i);
            scene->removeItem(u);
            delete u;
        }
    }

    const qreal enemyActiveRadius = 600.0;
    const qreal activeRadius2 = enemyActiveRadius * enemyActiveRadius;
    for (int i = 0; i < enemies.size(); ++i)
    {
        Enemy* e = enemies[i];
        if (!e) continue;
        QPointF d = e->pos() - cameraTarget;
        qreal dist2 = d.x()*d.x() + d.y()*d.y();
        if (dist2 <= activeRadius2)
        {
            e->updateMovement(playerPos);
        }
        else
        {
            e->idleUpdate();
        }
    }

    const qreal bossActiveRadius = 1200.0;
    const qreal bossActiveRadius2 = bossActiveRadius * bossActiveRadius;
    for (int i = 0; i < bosses.size(); ++i)
    {
        Boss* b = bosses[i];
        if (!b) continue;
        QPointF d = b->pos() - cameraTarget;
        qreal dist2 = d.x()*d.x() + d.y()*d.y();
        if (dist2 <= bossActiveRadius2)
            b->updateMovement(playerPos);
        else
            b->idleUpdate();
    }

    // Update XP orbs
    for (int i = 0; i < xpOrbs.size(); ++i)
    {
        xpOrbs[i]->moveTowardsPlayer(playerPos);
    }

    // Two-phase merging with variable combine size (15-35)
    const int minCombine = 15;
    const int maxCombine = 35;
    const qreal clusterRadius = 300.0;
    const qreal combineDistance = 12.0;

    bool startedClusterThisFrame = false;
    for (int i = 0; i < xpOrbs.size() && !startedClusterThisFrame; ++i)
    {
        XPOrb* a = xpOrbs[i];
        if (!a || a->isLocked() || a->isMerging()) continue;

        QList<XPOrb*> neighbors;
        neighbors.append(a);

        for (int j = 0; j < xpOrbs.size(); j++)
        {
            if (i == j) continue;
            XPOrb* b = xpOrbs[j];
            if (!b || b->isLocked() || b->isMerging()) continue;
            qreal dx = a->pos().x() - b->pos().x();
            qreal dy = a->pos().y() - b->pos().y();
            if (dx*dx + dy*dy <= clusterRadius * clusterRadius)
                neighbors.append(b);
        }

        int N = neighbors.size();
        if (N >= minCombine)
        {
            qreal total = 0.0;
            QPointF weighted(0,0);
            for (XPOrb* o : neighbors) {
                int v = o->getXPValue();
                total += v;
                weighted += o->pos() * v;
            }
            if (total <= 0) continue;
            QPointF centroid = weighted / total;

            for (XPOrb* o : neighbors) {
                o->startMerging(centroid);
            }

            startedClusterThisFrame = true;
        }
    }

    QMap<QPair<int,int>, QList<XPOrb*>> mergingGroups;
    for (XPOrb* o : xpOrbs)
    {
        if (!o || !o->isMerging()) continue;
        QPointF t = o->mergeTarget();
        int keyX = qRound(t.x() * 10.0);
        int keyY = qRound(t.y() * 10.0);
        mergingGroups[qMakePair(keyX, keyY)].append(o);
    }

    for (auto it = mergingGroups.begin(); it != mergingGroups.end(); ++it)
    {
        QList<XPOrb*> group = it.value();
        if (group.size() < minCombine) continue;

        QPointF target = group.first()->mergeTarget();
        QList<QPair<qreal,XPOrb*>> readyDistances;
        for (XPOrb* o : group)
        {
            qreal dx = o->pos().x() - target.x();
            qreal dy = o->pos().y() - target.y();
            qreal d2 = dx*dx + dy*dy;
            if (d2 <= combineDistance * combineDistance)
                readyDistances.append(qMakePair(d2, o));
        }

        if (readyDistances.size() >= minCombine)
        {
            std::sort(readyDistances.begin(), readyDistances.end(),
                      [](const QPair<qreal,XPOrb*> &a, const QPair<qreal,XPOrb*> &b){
                          return a.first < b.first;
                      });

            int readyCount = readyDistances.size();
            int combineCount = qBound(minCombine, readyCount, maxCombine);

            int totalXP = 0;
            QPointF weighted(0,0);

            QList<XPOrb*> toRemove;
            for (int k = 0; k < combineCount; ++k)
            {
                XPOrb* o = readyDistances[k].second;
                toRemove.append(o);
                totalXP += o->getXPValue();
                weighted += o->pos() * o->getXPValue();
            }

            if (totalXP > 0)
            {
                QPointF combinedPos = weighted / static_cast<qreal>(totalXP);

                for (XPOrb* o : toRemove)
                {
                    xpOrbs.removeOne(o);
                    if (o->scene()) scene->removeItem(o);
                    delete o;
                }

                XPOrb* combined = new XPOrb(combinedPos, totalXP);
                combined->setLocked(true);
                scene->addItem(combined);
                xpOrbs.append(combined);
            }

            for (XPOrb* o : group)
            {
                if (!toRemove.contains(o))
                    o->stopMerging();
            }
        }
    }

    // Check XP orb collection
    QList<XPOrb*> orbsToRemove;
    for (int i = 0; i < xpOrbs.size(); ++i)
    {
        if (player->collidesWithItem(xpOrbs[i]))
        {
            orbsToRemove.append(xpOrbs[i]);
        }
    }

    for (int i = 0; i < orbsToRemove.size(); ++i)
    {
        onXPOrbCollected(orbsToRemove[i]);
    }

    // Check space object collisions with player and enemies
    for (int i = 0; i < spaceObjects.size(); ++i)
    {
        if (!spaceObjects[i]->hasCollision())
            continue;

        if (player->collidesWithItem(spaceObjects[i]))
        {
            QPointF playerPos = player->pos();
            QPointF objPos = spaceObjects[i]->pos();
            QPointF pushDir = playerPos - objPos;
            player->pushBack(pushDir, 5.0);
        }

        for (int j = 0; j < enemies.size(); ++j)
        {
            if (enemies[j]->collidesWithItem(spaceObjects[i]))
            {
                QPointF enemyPos = enemies[j]->pos();
                QPointF objPos = spaceObjects[i]->pos();
                QPointF pushDir = enemyPos - objPos;
                qreal dx = pushDir.x();
                qreal dy = pushDir.y();
                qreal distance = qSqrt(dx * dx + dy * dy);
                if (distance > 0)
                {
                    enemies[j]->setPos(
                        enemyPos.x() + (dx / distance) * 3.0,
                        enemyPos.y() + (dy / distance) * 3.0
                        );
                }
            }
        }
    }

    collisionManager->checkCollisions(player, bullets, enemies, bosses, ultimates);

    // Update all bullets
    QList<Bullet*> bulletsToRemove;
    int viewW = viewport() ? viewport()->width() : 800;
    int viewH = viewport() ? viewport()->height() : 600;

    for (int i = 0; i < bullets.size(); ++i)
    {
        bullets[i]->move();
        
        if (collisionManager->isOffScreen(bullets[i], cameraTarget, viewW, viewH))
        {
            bulletsToRemove.append(bullets[i]);
        }
    }


    for (int i = player->getActiveGrenades().size() - 1; i >= 0; --i) {
        Grenades* g = player->getActiveGrenades()[i];
        g->move();

        if (g->getIsExploding()) {
            for (int j = enemies.size() - 1; j >= 0; --j) {
                Enemy* e = enemies[j];
                qreal dist = QLineF(g->pos(), e->pos()).length();

                if (dist < g->rect().width() / 2) {
                    onEnemyDestroyed(e);
                }
            }

            for (Boss* b : bosses) {
                if (QLineF(g->pos(), b->pos()).length() < g->rect().width() / 2) {
                    b->takeDamage(40);
                }
            }
        }

        if (g->isFinished()) {
            player->removeActiveGrenade(i);
            scene->removeItem(g);
            delete g;
        }
    }

    // Remove bullets that went off screen
    for (int i = 0; i < bulletsToRemove.size(); i++)
    {
        bullets.removeOne(bulletsToRemove[i]);
        scene->removeItem(bulletsToRemove[i]);
        delete bulletsToRemove[i];
    }

    static int sendCounter = 0;
    sendCounter++;

    if (sendCounter >= 12) {
        int bossStatus = 1;
        sendCounter = 0;
    }

    arduino->sendGameState(levelSystem->getLevel(), this->currentBossID, player->getIsUltimateReady());
    player->processMovement();
}

void Game::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    centerOn(cameraTarget);
    if (hud) hud->updatePosition(cameraTarget, player->pos());
    QGraphicsView::resizeEvent(event);
}

void Game::toggleFullscreen()
{
    if (!isFullscreen)
    {
        previousGeometry = geometry();
        showFullScreen();
        isFullscreen = true;
    }
    else
    {
        showNormal();
        if (!previousGeometry.isNull())
            setGeometry(previousGeometry);
        isFullscreen = false;
    }
}

/*void Game::onPauseMenuRequested()
{
    if (timer) timer->stop();
    m_isPaused = true;
    if (player) player->resetInputStates();
    if (m_pauseMenu) {
        m_pauseMenu->showPauseMenu(true);
        m_pauseMenu->raise();
        m_pauseMenu->setFocus();
    }
}*/
void Game::onPauseMenuRequested()
{
    if (m_isPaused || m_upgradeMenuOpen) return; // Évite les doubles ouvertures

    if (timer) timer->stop();
    m_isPaused = true;

    if (player) {
        player->resetInputStates();
        // 1. On déconnecte le joystick du vaisseau pour ne pas bouger en pause
        disconnect(arduino, &ArduinoManager::commandReceived, player, &Player::updateFromJoystick);
    }

    if (m_pauseMenu) {
        // 2. On connecte le joystick à la navigation du menu de pause
        // Note: Assure-toi que Menu possède une fonction navigateWithJoystick comme ton UpgradeMenu
        connect(arduino, &ArduinoManager::commandReceived, m_pauseMenu, &Menu::navigateWithJoystick, Qt::UniqueConnection);

        m_pauseMenu->showPauseMenu(true);
        m_pauseMenu->raise();
        m_pauseMenu->setFocus();
    }
}

void Game::onResumeGame()
{
    if (m_pauseMenu) {
        // 1. On déconnecte le joystick du menu de pause
        disconnect(arduino, &ArduinoManager::commandReceived, m_pauseMenu, &Menu::navigateWithJoystick);
	}
    m_isPaused = false;
    if (player) {
        connect(arduino, &ArduinoManager::commandReceived, player, &Player::updateFromJoystick);
    }
    if (timer) timer->start();
    setFocus();
}
