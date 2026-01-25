#include "levelsystem.h"

LevelSystem::LevelSystem(QObject *parent) : QObject(parent)
{
    level = 1;
    currentXP = 0;
    xpToNextLevel = calculateXPForLevel(2);
}

int LevelSystem::calculateXPForLevel(int targetLevel)
{
    // XP required grows exponentially: 100, 250, 450, 700, 1000...
    return 50 + (targetLevel - 1) * 150;
}

void LevelSystem::addXP(int xp)
{
    currentXP += xp;

    // Check for level up
    while (currentXP >= xpToNextLevel)
    {
        currentXP -= xpToNextLevel;
        level++;
        xpToNextLevel = calculateXPForLevel(level + 1);
        emit levelUp(level);
    }

    emit xpChanged(currentXP, xpToNextLevel);
}
