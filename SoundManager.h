#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <QObject>
#include <QSoundEffect>
#include <QMap>
#include <QString>

class SoundManager : public QObject
{
    Q_OBJECT
public:
    enum SoundType {
        PlayerShoot,
        PlayerHurt,
        PlayerUltimate,
        PlayerDeath,
        GrenadeThrow,
        GrenadeExplosion,
        UpgradeMenuAppear,
        UpgradeSelected
    };

    static SoundManager* instance();
    
    void playSound(SoundType type);
    void setVolume(int volumePercent); // 0-100
    
private:
    explicit SoundManager(QObject *parent = nullptr);
    ~SoundManager();
    
    void loadSounds();
    
    static SoundManager* s_instance;
    QMap<SoundType, QSoundEffect*> m_sounds;
    qreal m_volume; // 0.0 to 1.0
};

#endif // SOUNDMANAGER_H

