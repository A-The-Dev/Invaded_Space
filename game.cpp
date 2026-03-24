#include "game.h"
#include "upgrade_menu.h"
#include <QKeyEvent>
#include <QLineF>
#include <QRandomGenerator>
#include <QtMath>

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

    arduino = new ArduinoManager(this);
    arduino->connectToArduino("COM3");

    // 1. Reçoit les données de l'Arduino et fait bouger/tirer le joueur
    connect(arduino, &ArduinoManager::commandReceived, player, &Player::updateFromJoystick);

    // 2. Reçoit la balle créée par le joueur et l'ajoute à la liste de collision
    connect(player, &Player::bulletFired, this, [this](Bullet* b){
        scene->addItem(b);
        bullets.append(b);
    });

    // Initialize camera
    cameraTarget = player->pos();
    cameraSmoothing = 0.1;  // Lower = smoother/slower, Higher = faster
    centerOn(player);

    // Initialize spawn timer
    spawnTimer = 0;
    enemySpawnTimer = 0;

    // Create collision manager
    collisionManager = new CollisionManager(this);
    connect(collisionManager, &CollisionManager::enemyDestroyed, this, &Game::onEnemyDestroyed);
    connect(collisionManager, &CollisionManager::playerHit, this, &Game::onPlayerHit);

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

void Game::mouseMoveEvent(QMouseEvent *event)
{
    QPointF scenePos = mapToScene(event->pos());
    player->updateRotation(scenePos);
}

void Game::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        Bullet *bullet = new Bullet(player->pos(), player->getAngle(), true, player->getAttackDamage());
        scene->addItem(bullet);  // Add to scene directly, not as child of player
        bullets.append(bullet);
    }
}

void Game::keyPressEvent(QKeyEvent *event)
{
    player->keyPressEvent(event);
}

void Game::keyReleaseEvent(QKeyEvent *event)
{
    player->keyReleaseEvent(event);
}

void Game::onEnemyDestroyed(Enemy *enemy)
{
    // Spawn XP orb at enemy position
    int xpValue = 10;  // Base XP value
    XPOrb *orb = new XPOrb(enemy->pos(), 20);
    scene->addItem(orb);
    xpOrbs.append(orb);

    enemies.removeOne(enemy);
    scene->removeItem(enemy);
    enemy->deleteLater();

}

void Game::onPlayerHit(Enemy *enemy)
{
    // Player already gets pushed back in collision manager
    // Health is handled by player's takeDamage
}

void Game::onEnemyShoot(QPointF position, qreal angle)
{
    Bullet *bullet = new Bullet(position, angle, false);  // false = enemy bullet
    scene->addItem(bullet);
    bullets.append(bullet);
}

void Game::onPlayerDied()
{
    // Game over logic
    timer->stop();
    setWindowTitle("GAME OVER - Close to restart");
}

/*void Game::onLevelUp(int level)
{
    // Notification visuelle (titre de la fenêtre)
    setWindowTitle(QString("Invaded Space - Level %1!").arg(level));
    hud->updateLevel(levelSystem->getLevel());


    // 1. Santé : +2 points de vie max et on soigne tout
    player->increaseMaxHealth(2);
    player->refillHealth();


    // 3. PUISSANCE  : On augmente les dégâts de +1 à chaque montée de niveau
    int nouveauxDegats = player->getAttackDamage() + 1;
    player->setAttackDamage(nouveauxDegats);

    // Debug pour vérifier dans la console
    qDebug() << "LEVEL UP ! Niveau :" << level
             << "| Dégâts :" << player->getAttackDamage();
}*/
void Game::onLevelUp(int level)
{
    // 1. Mettre le jeu en pause
    timer->stop();

    // 2. Créer et afficher le menu
    UpgradeMenu *menu = new UpgradeMenu(this);

    // On connecte le choix du menu aux actions du joueur
    connect(menu, &UpgradeMenu::upgradeSelected, [this](int choice) {
        if (choice == 0) { // Vitesse
            //player->setSpeed(player->getSpeed() + 0.5);
        }
        else if (choice == 1) { // Dégâts
            player->setAttackDamage(player->getAttackDamage() + 1);
        }
        else if (choice == 2) { // Vie
            player->increaseMaxHealth(2);
            player->refillHealth();
        }
    });

    // 3. Exécuter le menu (Bloque ici jusqu'à ce qu'on clique)
    menu->exec();

    // 4. Relancer le jeu après la fermeture du menu
    timer->start();

    // Mise à jour du HUD
    hud->updateLevel(level);
    setWindowTitle(QString("Invaded Space - Level %1").arg(level));
    arduino->sendGameState(level, 0);
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
    hud->updatePosition(cameraTarget);

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
        enemySpawnTimer = 0;
        if (enemies.size() < 10 * levelSystem->getLevel())  // Cap at 10 enemies * current level
        {
            spawnEnemy();
        }
    }

    // Update enemies
    for (int i = 0; i < enemies.size(); ++i)
    {
        enemies[i]->updateMovement(playerPos);
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
    collisionManager->checkCollisions(player, bullets, enemies);

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
    player->updateFromJoystick(0.0, 0.0, true);
    // LIGNE DE TEST TEMPORAIRE :
    // On simule un mouvement vers la droite (0 rad) à pleine vitesse et un tir
    // player->updateFromJoystick(0.0, 1.0, false);
}
