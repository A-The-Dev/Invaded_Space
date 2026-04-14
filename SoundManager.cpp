#include "SoundManager.h"
#include <QDebug>
#include <QFileInfo>
#include <QDir>

SoundManager* SoundManager::s_instance = nullptr;

SoundManager* SoundManager::instance()
{
    if (!s_instance) {
        s_instance = new SoundManager();
    }
    return s_instance;
}

SoundManager::SoundManager(QObject *parent)
    : QObject(parent),
      m_volume(0.8) // Default 80% volume
{
    loadSounds();
}

SoundManager::~SoundManager()
{
    qDeleteAll(m_sounds);
    m_sounds.clear();
}

void SoundManager::loadSounds()
{
    // Define sound file paths
    QMap<SoundType, QString> soundPaths;
    soundPaths[PlayerShoot] = "shoot.wav";
    soundPaths[PlayerHurt] = "hurt.wav";
    soundPaths[PlayerUltimate] = "nuke.wav";
    soundPaths[PlayerDeath] = "death.wav";
    soundPaths[GrenadeThrow] = "grenadeThrow.wav";
    soundPaths[GrenadeExplosion] = "explosion.wav";
    soundPaths[UpgradeMenuAppear] = "upgrade.wav";
    soundPaths[UpgradeSelected] = "upgradeSelected.wav";
    
    // Load each sound effect
    for (auto it = soundPaths.constBegin(); it != soundPaths.constEnd(); ++it) {
        QSoundEffect* effect = new QSoundEffect(this);
        
        QString filePath = QString("./resources/%1").arg(it.value());
        
        // Convert to absolute path
        QFileInfo fileInfo(filePath);
        if (fileInfo.exists()) {
            QString absolutePath = fileInfo.absoluteFilePath();
            // Convert to proper file URL
            QUrl fileUrl = QUrl::fromLocalFile(absolutePath);
            effect->setSource(fileUrl);
            
            qDebug() << "SoundManager: Loaded:" << absolutePath;
        } else {
            qWarning() << "SoundManager: Failed to find sound file:" << filePath;
            qWarning() << "SoundManager: Current directory:" << QDir::currentPath();
        }
        
        effect->setVolume(m_volume);
        m_sounds[it.key()] = effect;
    }
}

void SoundManager::playSound(SoundType type)
{
    if (m_sounds.contains(type)) {
        QSoundEffect* effect = m_sounds[type];
        if (effect) {
            if (effect->isPlaying()) {
                effect->stop();
            }
            effect->play();
        }
    }
}

void SoundManager::setVolume(int volumePercent)
{
    // Convert 0-100 to 0.0-1.0
    m_volume = qBound(0.0, volumePercent / 100.0, 1.0);
    
    // Update volume for all loaded sounds
    for (QSoundEffect* effect : m_sounds) {
        if (effect) {
            effect->setVolume(m_volume);
        }
    }
}
