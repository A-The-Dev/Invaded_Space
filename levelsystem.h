#ifndef LEVELSYSTEM_H
#define LEVELSYSTEM_H

#include <QObject>

class LevelSystem : public QObject
{
    Q_OBJECT
public:
    LevelSystem(QObject *parent = nullptr);

    void addXP(int xp);
    void reset();
    int getLevel() const { return level; }
    int getCurrentXP() const { return currentXP; }
    int getXPForNextLevel() const { return xpToNextLevel; }

signals:
    void levelUp(int newLevel);
    void xpChanged(int currentXP, int xpToNextLevel);

private:
    int level;
    int currentXP;
    int xpToNextLevel;

    int calculateXPForLevel(int level);
};

#endif // LEVELSYSTEM_H
