#include "hud.h"
#include <QFont>
#include <QBrush>
#include <QPen>
#include <QTimer>

HUD::HUD(QGraphicsScene *scene, QObject *parent) : QObject(parent), scene(scene)
{
    QFont smallFont("Arial", 9, QFont::Bold);

    healthBarBackground = new QGraphicsRectItem();
    healthBarBackground->setRect(0, 0, 60, 6);
    healthBarBackground->setBrush(QBrush(QColor(0, 0, 0, 180)));
    healthBarBackground->setPen(Qt::NoPen);
    healthBarBackground->setZValue(2000);
    scene->addItem(healthBarBackground);

    healthBarFill = new QGraphicsRectItem();
    healthBarFill->setRect(0, 0, 60, 6);
    healthBarFill->setBrush(QBrush(QColor(50, 255, 50)));
    healthBarFill->setPen(Qt::NoPen);
    healthBarFill->setZValue(2001);
    scene->addItem(healthBarFill);

    healthText = new QGraphicsTextItem();
    healthText->setPlainText("20/20");
    healthText->setDefaultTextColor(Qt::white);
    healthText->setFont(smallFont);
    healthText->setZValue(2004);
    scene->addItem(healthText);

    levelBackground = new QGraphicsRectItem();
    levelBackground->setRect(0, 0, 36, 16);
    levelBackground->setBrush(Qt::NoBrush);
    levelBackground->setPen(Qt::NoPen);
    levelBackground->setZValue(1999);
    levelBackground->setVisible(false);
    scene->addItem(levelBackground);

    levelText = new QGraphicsTextItem();
    levelText->setPlainText("Lv 1");
    levelText->setDefaultTextColor(Qt::white);
    levelText->setFont(smallFont);
    levelText->setZValue(2004);
    scene->addItem(levelText);

    playerXPBackground = new QGraphicsRectItem();
    playerXPBackground->setRect(0, 0, 60, 4);
    playerXPBackground->setBrush(QBrush(QColor(0, 0, 0, 180)));
    playerXPBackground->setPen(Qt::NoPen);
    playerXPBackground->setZValue(2002);
    scene->addItem(playerXPBackground);

    playerXPFill = new QGraphicsRectItem();
    playerXPFill->setRect(0, 0, 0, 4);
    playerXPFill->setBrush(QBrush(QColor(100, 200, 255)));
    playerXPFill->setPen(Qt::NoPen);
    playerXPFill->setZValue(2003);
    scene->addItem(playerXPFill);

    ultimateBackground = new QGraphicsRectItem(0, 0, 60, 6);
    ultimateBackground->setBrush(QBrush(QColor(0,0,0,200)));
    ultimateBackground->setPen(Qt::NoPen);
    ultimateBackground->setZValue(1999);
    scene->addItem(ultimateBackground);

    ultimateBar = new QGraphicsRectItem(0, 0, 0, 6);
    ultimateBar->setBrush(QBrush(QColor(160, 64, 255)));
    ultimateBar->setPen(Qt::NoPen);
    ultimateBar->setZValue(2000);
    scene->addItem(ultimateBar);

    ultimatePulseTimer = new QTimer(this);
    ultimatePulseTimer->setInterval(160);
    connect(ultimatePulseTimer, &QTimer::timeout, this, &HUD::onUltimatePulse);
    ultimatePulseGrowing = false;
    
    grenadeAnimationTimer = new QTimer(this);
    grenadeAnimationTimer->setInterval(16);
    connect(grenadeAnimationTimer, &QTimer::timeout, this, &HUD::onGrenadeAnimationTick);
    grenadeAnimationTimer->start();
}

void HUD::createGrenadeSlots()
{
    // Clean up existing grenades if recreating
    for (QGraphicsRectItem* segment : grenadeSegments) {
        scene->removeItem(segment);
        delete segment;
    }
    for (QGraphicsRectItem* fill : grenadeFills) {
        scene->removeItem(fill);
        delete fill;
    }
    grenadeSegments.clear();
    grenadeFills.clear();
    grenadeFillProgress.clear();
    
    qreal slotWidth = 60.0 / maxGrenades;
    qreal gap = 1.0;
    qreal fillWidth = slotWidth - gap;
    qreal slotHeight = 4;
    
    for (int i = 0; i < maxGrenades; ++i) 
    {
        // Background slot
        QGraphicsRectItem* segment = new QGraphicsRectItem();
        segment->setRect(0, 0, fillWidth, slotHeight); 
        segment->setBrush(QBrush(QColor(100, 82, 0))); 
        segment->setPen(Qt::NoPen);
        segment->setZValue(2005);
        scene->addItem(segment);
        grenadeSegments.append(segment);
        
        // Fill overlay
        QGraphicsRectItem* fill = new QGraphicsRectItem();
        fill->setRect(0, 0, fillWidth, slotHeight);
        fill->setBrush(QBrush(QColor(255, 165, 0)));
        fill->setPen(Qt::NoPen);
        fill->setZValue(2006);
        scene->addItem(fill);
        grenadeFills.append(fill);
        
        // Track individual fill progress
        grenadeFillProgress.append(1.0f);
    }
}

void HUD::setMaxGrenades(int maxGrenadeCount)
{
    if (maxGrenadeCount <= 0) {
        return;
    }
    
    maxGrenades = maxGrenadeCount;
    currentGrenades = maxGrenades;
    regeneratingGrenadeIndex = -1;
    grenadeRegenProgress = 0.0f;
    
    createGrenadeSlots();
}

void HUD::updatePosition(QPointF cameraPos, QPointF playerPos)
{
    // 1. Setup constants for spacing
    qreal barWidth = 60;
    qreal barHeight = 6;
    qreal spacing = 4; // Space between elements
    qreal centerX = playerPos.x();

    // 2. Define Y-coordinates (Stacked from bottom to top)
    // We stack them upwards starting from above the player's head
    qreal currentY = playerPos.y() - 60;

    // XP Bar (Bottom of the stack)
    playerXPBackground->setPos(centerX - barWidth / 2, currentY + barHeight - 2);
    playerXPFill->setPos(centerX - barWidth / 2, currentY + barHeight - 2);
    currentY -= (barHeight - 5);

    // Health Bar
    healthBarBackground->setPos(centerX - barWidth / 2, currentY);
    healthBarFill->setPos(centerX - barWidth / 2, currentY);
    currentY -= (barHeight + spacing);

    // Ultimate Bar
    ultimateBackground->setPos(centerX - barWidth / 2, currentY);
    ultimateBar->setPos(centerX, (currentY + barHeight / 2) - 2);
    currentY -= (barHeight + 2);

    // Grenade Segments
    if (maxGrenades > 0) {
        qreal slotWidth = 60.0 / maxGrenades;
        qreal grenadeStartX = centerX - barWidth / 2;
        for (int i = 0; i < grenadeSegments.size(); ++i) {
            grenadeSegments[i]->setPos(grenadeStartX + (i * slotWidth), currentY);
            grenadeFills[i]->setPos(grenadeStartX + (i * slotWidth), currentY);
        }
        currentY -= (12 + spacing); // Move up for text
    }

    // 3. Text Placement (Placed at the very top to avoid overlap)
    // Center the Health and Level text side-by-side
    qreal healthW = healthText->boundingRect().width();
    qreal levelW = levelText->boundingRect().width();
    qreal totalTextW = healthW + levelW + 10;

    healthText->setPos(centerX - totalTextW / 2, currentY);
    levelText->setPos(centerX - totalTextW / 2 + healthW + 10, currentY);

    // Hide the level background if not used
    levelBackground->setVisible(false);
}

void HUD::updateGrenades(int count)
{
    if (maxGrenades == 0) {
        return;
    }
    
    currentGrenades = count;
    for (int i = 0; i < grenadeSegments.size(); ++i) 
    {
        if (i < count) 
        {
            // Fully charged grenades
            grenadeSegments[i]->setVisible(true);
            grenadeFills[i]->setOpacity(1.0);
            grenadeFills[i]->setRect(0, 0, grenadeSegments[i]->rect().width(), grenadeSegments[i]->rect().height());
            grenadeFillProgress[i] = 1.0f;
        }
        else if (i == count)
        {
            // Current regenerating grenade - reset its progress
            grenadeSegments[i]->setVisible(true);
            grenadeFillProgress[i] = 0.0f;
            regeneratingGrenadeIndex = i;
            grenadeRegenProgress = 0.0f;
        }
        else
        {
            // Empty grenades (not yet regenerating)
            grenadeSegments[i]->setVisible(true);
            grenadeFills[i]->setOpacity(0.4);
            grenadeFills[i]->setRect(0, 0, 0, grenadeSegments[i]->rect().height());
            grenadeFillProgress[i] = 0.0f;
        }
    }
}

void HUD::updateHealth(int health, int maxHealth)
{
    qreal barWidth = 60;
    qreal barHeight = 6;

    qreal healthPercent = (maxHealth > 0) ? static_cast<qreal>(health) / maxHealth : 0.0;
    healthBarFill->setRect(0, 0, barWidth * healthPercent, barHeight);

    if (healthPercent > 0.6)
        healthBarFill->setBrush(QBrush(QColor(50, 255, 50)));
    else if (healthPercent > 0.3)
        healthBarFill->setBrush(QBrush(QColor(255, 200, 50)));
    else
        healthBarFill->setBrush(QBrush(QColor(255, 50, 50)));

    healthText->setPlainText(QString("%1/%2").arg(health).arg(maxHealth));
}

void HUD::updateLevel(int level)
{
    levelText->setPlainText(QString("Lv %1").arg(level));
}

void HUD::updateXP(int currentXP, int xpToNextLevel)
{
    qreal xpPercent = (xpToNextLevel > 0) ? static_cast<qreal>(currentXP) / xpToNextLevel : 0.0;
    qreal barWidth = 60.0;
    qreal xpThin = 4.0;

    playerXPFill->setRect(0, 0, barWidth * qBound(0.0f, static_cast<float>(xpPercent), 1.0f), xpThin);
}

void HUD::updateUltimate(float percentage)
{
    qreal barWidth = 60;
    qreal barHeight = 6;
    qreal filledWidth = barWidth * qBound(0.0f, percentage, 1.0f);

    ultimateBar->setRect(-filledWidth / 2, 0, filledWidth, barHeight);

    if (percentage >= 1.0f)
    {
        if (!ultimatePulseTimer->isActive()) {
            ultimatePulseGrowing = false;
            ultimatePulseTimer->start();
        }
    }
    else
    {
        if (ultimatePulseTimer->isActive()) {
            ultimatePulseTimer->stop();
            ultimateBar->setOpacity(1.0);
            ultimateBar->setScale(1.0);
        }
        ultimateBar->setBrush(QBrush(QColor(160, 64, 255)));
    }
}

void HUD::onUltimatePulse()
{
    if (ultimatePulseGrowing) {
        ultimateBar->setScale(1.0);
        ultimateBar->setOpacity(1.0);
        ultimateBar->setBrush(QBrush(QColor(210, 170, 255)));
    } else {
        ultimateBar->setScale(1.08);
        ultimateBar->setOpacity(0.92);
        ultimateBar->setBrush(QBrush(QColor(180, 120, 255)));
    }
    ultimatePulseGrowing = !ultimatePulseGrowing;
}

void HUD::onGrenadeAnimationTick()
{
    if (regeneratingGrenadeIndex < 0 || regeneratingGrenadeIndex >= grenadeFills.size()) {
        return;
    }
    
    if (maxGrenades == 0) {
        return;
    }
    
    qreal slotWidth = 60.0 / maxGrenades;
    qreal gap = 1.0;
    qreal fillWidth = slotWidth - gap;
    qreal slotHeight = 4;
    qreal animationDuration = 10.0;
    qreal tickDuration = 0.016;
    qreal increment = tickDuration / animationDuration;
    
    grenadeRegenProgress += increment;
    if (grenadeRegenProgress >= 1.0f) {
        grenadeRegenProgress = 1.0f;
        // Fill the current grenade completely
        grenadeFills[regeneratingGrenadeIndex]->setRect(0, 0, fillWidth, slotHeight);
        grenadeFills[regeneratingGrenadeIndex]->setOpacity(1.0);
        grenadeFillProgress[regeneratingGrenadeIndex] = 1.0f;
        
        // Move to the next grenade if available
        if (regeneratingGrenadeIndex + 1 < maxGrenades) {
            regeneratingGrenadeIndex++;
            grenadeRegenProgress = 0.0f;
        } else {
            // All grenades fully charged
            regeneratingGrenadeIndex = -1;
        }
        return;
    }
    
    qreal currentFillWidth = fillWidth * grenadeRegenProgress;
    grenadeFills[regeneratingGrenadeIndex]->setRect(0, 0, currentFillWidth, slotHeight);
    grenadeFillProgress[regeneratingGrenadeIndex] = grenadeRegenProgress;
}
