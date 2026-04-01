#include "game.h"
#include <QKeyEvent>
#include <QLineF>
#include <QRandomGenerator>
#include <QtMath>
#include <QDebug>

Game::Game(QWidget *parent) : QGraphicsView(parent)
{
    // Create scene with larger area for free movement
    scene = new QGraphicsScene(this);
    scene->setSceneRect(-1000, -1000, 2000, 2000);

    // Set space-themed background
    scene->setBackgroundBrush(QBrush(QColor(10, 10, 30)));

    setScene(scene);

    // Set view properties
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFixedSize(800, 600);

    // Enable mouse tracking
    setMouseTracking(true);

    // Create player at origin
    player = new Player();
    player->setPos(0, 0);
    scene->addItem(player);

    // Initialize camera
    cameraTarget = player->pos();
    cameraSmoothing = 0.1;  // Lower = smoother/slower, Higher = faster
    centerOn(player);

    // Initialize spawn timer
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
        hud->updateHealth(health, maxHealth);
    });

    // Create HUD
    hud = new HUD(scene, this);
    hud->updateHealth(player->getHealth(), player->getMaxHealth());

    // Create level system
    levelSystem = new LevelSystem(this);
    connect(levelSystem, &LevelSystem::levelUp, this, &Game::onLevelUp);
    connect(levelSystem, &LevelSystem::xpChanged, hud, &HUD::updateXP);
    hud->updateLevel(levelSystem->getLevel());
    hud->updateXP(levelSystem->getCurrentXP(), levelSystem->getXPForNextLevel());

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

    // Create timer for game loop
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Game::updateGame);
    timer->start(16); // ~60 FPS

    // Set window title
    setWindowTitle("Invaded Space - WASD to move, Mouse to aim, Click to shoot");
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

    if (typeRoll < 33)
        type = Boss::Boss1;
    else if (typeRoll < 66)
        type = Boss::Boss2;
    else
        type = Boss::Boss4;

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
        Bullet *bullet = new Bullet(player->pos(), player->getAngle());
        scene->addItem(bullet);  // Add to scene directly, not as child of player
        bullets.append(bullet);
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
}

void Game::triggerScreenClear()
{

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


    QPixmap ultImg("../../resources/Léanuke.png");
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


}
void Game::keyReleaseEvent(QKeyEvent *event)
{
    player->keyReleaseEvent(event);
}

void Game::onEnemyDestroyed(Enemy *enemy)
{
    // Spawn XP orb at enemy position
    int xpValue = 10;  // Base XP value
    XPOrb *orb = new XPOrb(enemy->pos(), xpValue);
    scene->addItem(orb);
    xpOrbs.append(orb);

    enemies.removeOne(enemy);
    scene->removeItem(enemy);
    enemy->deleteLater();
    player->gainUltimateCharge(5.0); //Un ultimate par 20 enemy
    hud->updateUltimate(player->getUltimatePercentage());
}
void Game::onBossDestroyed(Boss *boss)
{
    // Spawn XP orb at enemy position
    int xpValue = 500;  // Base XP value
    XPOrb *orb = new XPOrb(boss->pos(), xpValue);
    scene->addItem(orb);
    xpOrbs.append(orb);

    bosses.removeOne(boss);
    scene->removeItem(boss);
    boss->deleteLater();
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

void Game::onBossUltimate(QPointF position, qreal angle, bool isBoss)
{

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
    // Game over logic
    timer->stop();
    setWindowTitle("GAME OVER - Close to restart");
}

void Game::onLevelUp(int level)
{
    // Show level up notification
    setWindowTitle(QString("Invaded Space - Level %1!").arg(level));
    hud->updateLevel(levelSystem->getLevel());
    // Could add level up effects, bonuses, etc.
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
    // Update enemies
    for (int i = 0; i < enemies.size(); ++i)
    {
        enemies[i]->updateMovement(playerPos);
    }
    // Update bosses
    for (int i = 0; i < bosses.size(); ++i)
    {
        bosses[i]->updateMovement(playerPos);
    }

    // Update XP orbs
    for (int i = 0; i < xpOrbs.size(); ++i)
    {
        xpOrbs[i]->moveTowardsPlayer(playerPos);
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
    for (int i = 0; i < bullets.size(); ++i)
    {
        bullets[i]->move();

        // Check if bullet is off screen
        if (collisionManager->isOffScreen(bullets[i], cameraTarget, 800, 600))
        {
            bulletsToRemove.append(bullets[i]);
        }
    }

    // Remove bullets that went off screen
    for (int i = 0; i < bulletsToRemove.size(); ++i)
    {
        bullets.removeOne(bulletsToRemove[i]);
        scene->removeItem(bulletsToRemove[i]);
        delete bulletsToRemove[i];
    }
}

