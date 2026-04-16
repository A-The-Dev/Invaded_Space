#pragma once

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <QList>
#include <QVector>
#include <QPropertyAnimation>

class SpaceObject;
class QPushButton;
class QLabel;
class QSlider;
class QCheckBox;
class Leaderboard;

class Menu : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal titleGlow READ titleGlow WRITE setTitleGlow)
public:
    explicit Menu(QWidget *parent = nullptr);
    ~Menu() override;

    // Pause menu overlay
    void showPauseMenu(bool show);
    bool isPauseMenuVisible() const;

    qreal titleGlow() const { return m_titleGlow; }
    void setTitleGlow(qreal glow);

signals:
    void startGameRequested();
    void fullscreenToggled(bool fullscreen);
    void volumeChanged(int value);
    void resumeGameRequested();

private slots:
    void onStartClicked();
    void onOptionsClicked();
    void onQuitClicked();
    void onBackFromOptions();
    void onVolumeChanged(int value);
    void onFullscreenChanged(bool checked);
    void onUpdateScene();
    void showLeaderboard(bool show);
    void onResumeClicked();
    void onPauseOptionsClicked();
    void onBackFromPauseOptions();
    void onPauseQuitClicked();

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void setupUI();
    void setupPauseMenuUI();
    void spawnSpaceObject();
    void loadPixelFontIfAvailable();
    void layoutResponsive();
    void setupTitleAnimation();

    enum Page { MainPage = 0, OptionsPage = 1, PauseMenuPage = 2, PauseOptionsPage = 3 };
    Page m_currentPage;

    QGraphicsView *m_view;
    QGraphicsScene *m_scene;
    QTimer *m_timer;
    QList<SpaceObject*> m_spaceObjects;

    // Overlay widgets
    QWidget *m_overlayWidget;
    QWidget *m_mainMenuWidget;
    QWidget *m_optionsWidget;
    QWidget *m_pauseMenuWidget;
    QWidget *m_pauseOptionsWidget;

    // Main menu controls
    QLabel *m_titleLabel;
    QPushButton *m_startButton;
    QPushButton *m_optionsButton;
    QPushButton *m_leaderboardButton;
    QPushButton *m_quitButton;

    // Pause menu controls
    QPushButton *m_resumeButton;
    QPushButton *m_pauseOptionsButton;
    QPushButton *m_pauseQuitButton;

    // Options controls
    QSlider *m_volumeSlider;
    QLabel *m_volumePercentLabel;
    QCheckBox *m_fullscreenCheck;
    QPushButton *m_optionsBackButton;
    QWidget* m_volumeContainer;

    // Pause options controls
    QSlider *m_pauseVolumeSlider;
    QLabel *m_pauseVolumePercentLabel;
    QCheckBox *m_pauseFullscreenCheck;
    QPushButton *m_pauseOptionsBackButton;
    QWidget* m_pauseVolumeContainer;

    // Navigation / selector
    QLabel *m_selectorLabel;
    QVector<QWidget*> m_mainItems;
    QVector<QWidget*> m_optionsItems;
    QVector<QWidget*> m_pauseItems;
    QVector<QWidget*> m_pauseOptionsItems;
    int m_selectedIndex;

    Leaderboard *m_leaderboard;
    QWidget *m_modalBackdrop;

    // Animation
    QPropertyAnimation *m_titleGlowAnimation;
    qreal m_titleGlow;

    // State
    int m_spawnTimer;
    bool m_usingPixelFont;
    bool m_isPauseMenuVisible;
    bool m_isMainMenu;

    void updateSelectorPosition();
    void moveSelection(int delta);
    void activateSelected();
    void ensureMenuFocus();
};

