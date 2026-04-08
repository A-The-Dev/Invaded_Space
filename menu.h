#pragma once

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <QList>
#include <QVector>

class SpaceObject;
class QPushButton;
class QLabel;
class QSlider;
class QCheckBox;
class Leaderboard;

class Menu : public QWidget
{
    Q_OBJECT
public:
    explicit Menu(QWidget *parent = nullptr);
    ~Menu() override;

signals:
    void startGameRequested();
    void fullscreenToggled(bool fullscreen);
    void volumeChanged(int value);

private slots:
    void onStartClicked();
    void onOptionsClicked();
    void onQuitClicked();
    void onBackFromOptions();
    void onVolumeChanged(int value);
    void onFullscreenChanged(bool checked);
    void onUpdateScene();
    void showLeaderboard(bool show);

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void setupUI();
    void spawnSpaceObject();
    void loadPixelFontIfAvailable();
    void layoutResponsive();

    enum Page { MainPage = 0, OptionsPage = 1 };
    Page m_currentPage;

    QGraphicsView *m_view;
    QGraphicsScene *m_scene;
    QTimer *m_timer;
    QList<SpaceObject*> m_spaceObjects;

    // Overlay widgets
    QWidget *m_overlayWidget;
    QWidget *m_mainMenuWidget;
    QWidget *m_optionsWidget;

    // Main menu controls
    QLabel *m_titleLabel;
    QPushButton *m_startButton;
    QPushButton *m_optionsButton;
    QPushButton *m_leaderboardButton;
    QPushButton *m_quitButton;

    // Options controls
    QSlider *m_volumeSlider;
    QLabel *m_volumePercentLabel;
    QCheckBox *m_fullscreenCheck;
    QPushButton *m_optionsBackButton;

    // Navigation / selector
    QLabel *m_selectorLabel;                 // ship icon or marker
    QVector<QWidget*> m_mainItems;           // orderable focusable widgets (main)
    QVector<QWidget*> m_optionsItems;        // orderable focusable widgets (options)
    int m_selectedIndex;

    Leaderboard *m_leaderboard;
    QWidget *m_modalBackdrop; // darkened backdrop when leaderboard visible

    // State
    int m_spawnTimer;
    bool m_usingPixelFont;

    void updateSelectorPosition();
    void moveSelection(int delta);
    void activateSelected();
    void ensureMenuFocus();
};

