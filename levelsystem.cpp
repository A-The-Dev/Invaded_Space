#include "levelsystem.h"

LevelSystem::LevelSystem(QObject *parent) : QObject(parent)
{
    level = 1;
    currentXP = 0;
    xpToNextLevel = calculateXPForLevel(2);
}

int LevelSystem::calculateXPForLevel(int targetLevel)
{
    return 10 + (targetLevel - 1) * 50;
}

void LevelSystem::addXP(int xp)
{
    currentXP += xp;

    while (currentXP >= xpToNextLevel)
    {
        currentXP -= xpToNextLevel;
        level++;
        xpToNextLevel = calculateXPForLevel(level + 1);
        emit levelUp(level);
    }

    emit xpChanged(currentXP, xpToNextLevel);
}

void LevelSystem::reset()
{
    level = 1;
    currentXP = 0;
    xpToNextLevel = calculateXPForLevel(2);
    emit xpChanged(currentXP, xpToNextLevel);
}
