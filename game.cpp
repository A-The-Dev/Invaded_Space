#include "game.h"
#include "upgrade_menu.h"
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
    arduino(nullptr), m_gameStarted(false)
{
    // Create scene with larger area for free movement
    scene = new QGraphicsScene(this);
    scene->setSceneRect(-1000, -1000, 2000, 2000);
    hud = new HUD(scene, this);
    // Set space-themed background
    scene->setBackgroundBrush(QBrush(QColor(10, 10, 30)));

    setScene(scene);

    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setCacheMode(QGraphicsView::CacheBackground);
    setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing, true);

    // Set view properties
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    resize(800, 600);

    // Enable mouse tracking
    setMouseTracking(true);

    // Create player at origin
    player = new Player();
    player->setPos(0, 0);
    scene->addItem(player);

    // Provide player with pointers to the current enemy / boss lists so auto-targeting works.
    player->setEnemyLists(&enemies, &bosses);

    arduino = new ArduinoManager(this);
    // attempt connection if desired (safe to leave)
    if (arduino->connectToArduino("COM3")) {
        qDebug() << "Connexion Arduino REUSSIE sur COM3";
        player->setUseJoystick(true);
    } else {
        qDebug() << "Connexion Arduino ECHOUEE. Mode clavier actif.";
        player->setUseJoystick(false);
    }

    // Receive Arduino commands (if connected)
    connect(arduino, &ArduinoManager::commandReceived, this, [this](double x, double y, bool tir, bool ulti) {
        player->updateFromJoystick(x, y, tir, ulti);
    });

    connect(player, &Player::grenadeCountChanged, hud, &HUD::updateGrenades);

    hud->updateGrenades(player->getGrenadeCount());


    // 2. Reçoit la balle créée par le joueur et l'ajoute à la liste de collision
    connect(player, &Player::bulletFired, this, [this](Bullet* b){
        scene->addItem(b);
        bullets.append(b);
    });
    connect(player, &Player::requestUltimate, this, &Game::triggerScreenClear);

	connect(player, &Player::grenadeThrown, this, [this](Grenades* g) 
        {
            scene->addItem(g);
        });
    // Initialize camera
    cameraTarget = player->pos();
    cameraSmoothing = 0.1;  // Lower = smoother/slower, Higher = faster
    centerOn(player);

    // Initialize spawn timer counters
    spawnTimer = 0;
    enemySpawnTimer = 0;
    bossSpawnTimer = 0;

    // Create collision manager
    collisionManager = new CollisionManager(this);
    connect(collisionManager, &CollisionManager::enemyDestroyed, this, &Game::onEnemyDestroyed);
    connect(collisionManager, &CollisionManager::playerHitEnemy, this, &Game::onPlayerHit);
    connect(collisionManager, &CollisionManager::playerHitBoss, this, &Game::onPlayerHit);



    // Connect player signals
    connect(player, &Player::died, this, &Game::onPlayerDied);
    connect(player, &Player::healthChanged, this, [this](int health, int maxHealth) {
        if (hud) hud->updateHealth(health, maxHealth);
    });

    // Create HUD

    hud->updateHealth(player->getHealth(), player->getMaxHealth());

    // Create level system
    levelSystem = new LevelSystem(this);
    connect(levelSystem, &LevelSystem::levelUp, this, &Game::onLevelUp);
    connect(levelSystem, &LevelSystem::xpChanged, hud, &HUD::updateXP);
    hud->updateLevel(levelSystem->getLevel());
    hud->updateXP(levelSystem->getCurrentXP(), levelSystem->getXPForNextLevel());

    // If not paused, start gameplay immediately, otherwise wait for explicit startGame() call.
    if (!startPaused) {
        startGame();
    }
}

void Game::startGame()
{
    if (m_gameStarted) return;
    m_gameStarted = true;

    // Spawn initial space objects
    for (int i = 0; i < 50; ++i)
    {
        spawnSpaceObject();
    }

    // Spawn initial enemies
    for (int i = 0; i < 6; ++i)
    {
        spawnEnemy();
    }
	player->setGrenadeCount(3); // Start with 3 grenades

    // Create timer for game loop and start it
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Game::updateGame);
    timer->start(16); // ~60 FPS

    // Set window title
    setWindowTitle("Invaded Space - WASD to move, Click to shoot");
}


void Game::spawnSpaceObject()
{
    QRandomGenerator *rng = QRandomGenerator::global();

    // Random position
    qreal x = rng->bounded(-1000, 1000);
    qreal y = rng->bounded(-1000, 1000);

    // Random type (more stars than planets/asteroids)
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

    // Random enemy type
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

    // Random boss type
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
        // Use Player::shoot() so auto-aim is applied and rate limiting is respected.
        player->shoot();
    }
    else if(event->button() == Qt::RightButton)
    {
        player->throwGrenade();
	}
}

void Game::keyPressEvent(QKeyEvent *event)
{
    player->keyPressEvent(event);
    if (event->key() == Qt::Key_F)
    {
        if (player->tryUseUltimate()) {
            triggerScreenClear();
        }
    }
    

    // Toggle fullscreen with F11
    if (event->key() == Qt::Key_F11)
    {
        toggleFullscreen();
    }
}

void Game::triggerScreenClear()
{
	UltUsed = true;
    while (!enemies.isEmpty())
    {
        Enemy* e = enemies.takeFirst();


        if (scene->views().first()->sceneRect().contains(e->pos()))
        {

            onEnemyDestroyed(e);
        } else
        {

            scene->removeItem(e);
            e->deleteLater();
        }
    }


    for (Boss* b : bosses)
    {
        b->takeDamage(100);
    }


    QPixmap ultImg("./resources/Léanuke.png");
    QGraphicsPixmapItem* flash = new QGraphicsPixmapItem(ultImg);


    flash->setOffset(-ultImg.width()/2, -ultImg.height()/2);
    flash->setPos(player->pos());

    flash->setZValue(2000);
    flash->setOpacity(0.8);

    scene->addItem(flash);

    // Fade it out or just remove it
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
    // Spawn XP orb at enemy position
    int xpValue = 20;  // Base XP value
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
        player->gainUltimateCharge(5.0); //Un ultimate par 20 enemy
        hud->updateUltimate(player->getUltimatePercentage());
    }

}
void Game::onBossDestroyed(Boss *boss)
{
    currentScore += 100;
    // Spawn XP orb at enemy position
    int xpValue = 500;  // Base XP value
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
        player->gainUltimateCharge(5.0); //Un ultimate par 20 enemy
        hud->updateUltimate(player->getUltimatePercentage());
    }
}

void Game::onPlayerHit(Enemy *enemy)
{
    // Player already gets pushed back in collision manager
    // Health is handled by player's takeDamage
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
        Bullet *bullet = new Bullet(position, angle, false);  // false = enemy bullet
        scene->addItem(bullet);
        bullets.append(bullet);
    }
}

void Game::onBossUltimate(QPointF position, qreal angle, bool isBoss) {
    Boss* senderBoss = qobject_cast<Boss*>(sender());



    if (senderBoss->getType() == Boss::Boss1) {
        // 8-Way Burst
        for (int i = 0; i < 8; ++i) {
            Ultimate *u = new Ultimate(position, i * 45, false, nullptr, senderBoss, true);
            scene->addItem(u);
            ultimates.append(u);
        }
    }
    else if (senderBoss->getType() == Boss::Boss2) {
        // The Wall: Calculate perpendicular angle in Radians
        qreal rad = (angle + 90) * M_PI / 180.0;

        for (int i = -2; i <= 2; ++i) {
            qreal spacing = i * 60; // Increased spacing for visibility
            QPointF wallPos = position + QPointF(qCos(rad) * spacing, qSin(rad) * spacing);

            Ultimate *u = new Ultimate(wallPos, angle, false, nullptr, senderBoss, true);
            scene->addItem(u);
            ultimates.append(u);
        }
    }
    else if (senderBoss->getType() == Boss::Boss4) {
        // The Giant Laser
        Ultimate *laser = new Ultimate(position, angle, false, nullptr, senderBoss, true);
        scene->addItem(laser);
        ultimates.append(laser);

        // Laser is temporary: delete after 600ms
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
    JsonParser::savePlayerResult(player, levelSystem->getLevel(),  currentScore);
   

    Leaderboard* lb = new Leaderboard();
    QGraphicsProxyWidget* proxy = scene->addWidget(lb);

    // Positionnement
    proxy->setPos(cameraTarget.x() - lb->width() / 2, cameraTarget.y() - lb->height() / 2);
    proxy->setZValue(10000);

    // --- RENDRE LE WIDGET PRIORITAIRE ---
    lb->setFocusPolicy(Qt::StrongFocus);
    lb->setFocus();

    // Optionnel : Désactiver le scroll de la vue principale pendant que le leaderboard est là
    this->setTransformationAnchor(QGraphicsView::NoAnchor);
}

void Game::onLevelUp(int level) {
    timer->stop();

    if (player) player->resetInputStates();

    // On déconnecte le joystick du joueur 
    disconnect(arduino, &ArduinoManager::commandReceived, player, &Player::updateFromJoystick);

    // Créer et afficher le menu
    UpgradeMenu* menu = new UpgradeMenu(this);

    // On connecte la manette au menu AVANT qu'il ne s'affiche
    connect(arduino, &ArduinoManager::commandReceived, menu, &UpgradeMenu::navigateWithJoystick);

    // Connecte le choix du menu aux actions du joueur
    connect(menu, &UpgradeMenu::upgradeSelected, [this](int choice) {
        if (choice == 0) { // Vitesse
            qreal increment = 0.4; // smaller, reasonable increment
            player->setSpeed(player->getSpeed() + increment);
            qDebug() << "Applied speed upgrade, new speed =" << player->getSpeed();
        }
        else if (choice == 1) { // Dégâts
            player->setAttackDamage(player->getAttackDamage() + 1);
            qDebug() << "Applied damage upgrade, new damage =" << player->getAttackDamage();
        }
        else if (choice == 2) { // Vie
            player->increaseMaxHealth(2);
        }
        player->refillHealth();
        });

    // Affiche le menu 
    menu->exec();

    // On déconnecte la manette du menu
    disconnect(arduino, &ArduinoManager::commandReceived, menu, &UpgradeMenu::navigateWithJoystick);

    // On redonne le contrôle du joystick au joueur
    connect(arduino, &ArduinoManager::commandReceived, player, &Player::updateFromJoystick);

    // Relance le jeu
    timer->start();

    // Mise à jour du HUD
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
    // Update player
    player->update();
    // Smooth camera follow
    QPointF playerPos = player->pos();
    cameraTarget.setX(cameraTarget.x() + (playerPos.x() - cameraTarget.x()) * cameraSmoothing);
    cameraTarget.setY(cameraTarget.y() + (playerPos.y() - cameraTarget.y()) * cameraSmoothing);
    centerOn(cameraTarget);

    // Update HUD position
    hud->updatePosition(cameraTarget, player->pos());

    // Update space objects and remove expired ones
    QList<SpaceObject*> objectsToRemove;
    for (int i = 0; i < spaceObjects.size(); ++i)
    {
        spaceObjects[i]->update();

        // Check if lifetime expired
        if (spaceObjects[i]->property("lifetime").toReal() <= 0)
        {
            objectsToRemove.append(spaceObjects[i]);
        }
    }

    // Remove expired objects
    for (int i = 0; i < objectsToRemove.size(); ++i)
    {
        spaceObjects.removeOne(objectsToRemove[i]);
        scene->removeItem(objectsToRemove[i]);
        delete objectsToRemove[i];
    }

    // Spawn new objects periodically
    spawnTimer++;
    if (spawnTimer >= 120)  // Spawn every 2 seconds (at 60fps)
    {
        spawnTimer = 0;
        spawnSpaceObject();
    }

    // Spawn enemies periodically
    enemySpawnTimer++;
    if (enemySpawnTimer >= 60)  // Spawn every second
    {
        if (enemies.size() < 5 * levelSystem->getLevel())  // Cap at 10 enemies * current level
        {
            spawnEnemy();
        }
    }

    int currentLevel = levelSystem->getLevel();

    if (currentLevel > 0 && currentLevel % 5 == 0) {
        // Only spawn if we haven't spawned one for THIS level yet
        if (bosses.isEmpty() && lastBossLevel != currentLevel) {
            spawnBoss();
            lastBossLevel = currentLevel; // Lock spawning for this level
        }
    }
    for (int i = ultimates.size() - 1; i >= 0; --i)
    {
        ultimates[i]->move();

        // Cleanup if they go too far (except for the static Boss 4 laser)
        if (ultimates[i]->getSpeed() > 0 && collisionManager->isOffScreen(ultimates[i], cameraTarget, 2000, 2000)) {
            Ultimate* u = ultimates.takeAt(i);
            scene->removeItem(u);
            delete u;
        }
    }

    // Update enemies with LOD: full update only when near camera; idleUpdate otherwise.
    const qreal enemyActiveRadius = 600.0; // tuneable radius around cameraTarget for full updates
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
    // Update bosses: fewer bosses expected so keep full updates but still skip if extremely far
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
            b->idleUpdate(); // if Boss has no idleUpdate, this is a no-op; add similar method if needed
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

    // Phase 1: detect clusters and start merging (one cluster per frame)
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

    // Phase 2: when enough orbs reach centroid, combine a bounded number (15-35)
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

            // reset merging flag on remaining orbs in this group so they don't get stuck
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

        // Check player collision
        if (player->collidesWithItem(spaceObjects[i]))
        {
            QPointF playerPos = player->pos();
            QPointF objPos = spaceObjects[i]->pos();
            QPointF pushDir = playerPos - objPos;
            player->pushBack(pushDir, 5.0);
        }

        // Check enemy collisions
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

    // Check collisions
    collisionManager->checkCollisions(player, bullets, enemies, bosses, ultimates);

    // Update all bullets (use index-based loop to avoid detachment warning)
    QList<Bullet*> bulletsToRemove;
    int viewW = viewport() ? viewport()->width() : 800;
    int viewH = viewport() ? viewport()->height() : 600;

    for (int i = 0; i < bullets.size(); ++i)
    {
        bullets[i]->move();
        
        // Check if bullet is off screen
        if (collisionManager->isOffScreen(bullets[i], cameraTarget, viewW, viewH))
        {
            bulletsToRemove.append(bullets[i]);
        }
    }


    for (int i = player->getActiveGrenades().size() - 1; i >= 0; --i) {
        Grenades* g = player->getActiveGrenades()[i];
        g->move();

        if (g->getIsExploding()) {
            // Area of Effect (AoE) Damage Logic
            for (int j = enemies.size() - 1; j >= 0; --j) {
                Enemy* e = enemies[j];
                // Check distance between grenade center and enemy
                qreal dist = QLineF(g->pos(), e->pos()).length();

                if (dist < g->rect().width() / 2) { // Inside explosion radius
                    onEnemyDestroyed(e); // Or e->takeDamage(40) if enemies have HP
                }
            }

            // Also check Bosses
            for (Boss* b : bosses) {
                if (QLineF(g->pos(), b->pos()).length() < g->rect().width() / 2) {
                    b->takeDamage(40);
                }
            }
        }

        // Cleanup: If the grenade object was marked for deletion by its own timer
        // Note: It's safer to check g->scene() == nullptr or a custom flag
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
    // On envoie les données environ 5 fois par seconde (tous les 12 cycles de 16ms)
    static int sendCounter = 0;
    sendCounter++;

    if (sendCounter >= 12) {
        //int currentLevel = levelSystem->getLevel();

        // Logique pour le chiffre du boss (1 à 4)
        // Ici, tu peux mettre ta propre logique. Exemple :
        // 1 = Pas de boss, 2 = Petit boss, 3 = Gros boss, 4 = Boss final
        int bossStatus = 1;
        //if (enemies.size() > 15) bossStatus = 2; // Exemple simple

        //arduino->sendGameState(currentLevel, bossStatus);
        sendCounter = 0;
    }

    //arduino->sendGameState(3,3, true );
    arduino->sendGameState(levelSystem->getLevel(), this->currentBossID, player->getIsUltimateReady());
    player->processMovement(); // On bouge le joueur à chaque "tick" du timer
    //player->updateFromJoystick(0.0, 0.0, true);
    // LIGNE DE TEST TEMPORAIRE :
    // On simule un mouvement vers la droite (0 rad) à pleine vitesse et un tir
    // player->updateFromJoystick(0.0, 1.0, false);
}

void Game::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    // Keep camera centered and update HUD when the view size changes
    centerOn(cameraTarget);
    if (hud) hud->updatePosition(cameraTarget, player->pos());
    QGraphicsView::resizeEvent(event);
}

void Game::toggleFullscreen()
{
    if (!isFullscreen)
    {
        // enter fullscreen: save previous geometry to restore later
        previousGeometry = geometry();
        showFullScreen();
        isFullscreen = true;
    }
    else
    {
        // exit fullscreen: restore previous windowed geometry
        showNormal();
        if (!previousGeometry.isNull())
            setGeometry(previousGeometry);
        isFullscreen = false;
    }
}
