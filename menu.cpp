#include "menu.h"
#include "spaceobject.h"
#include "Leaderboard.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QRandomGenerator>
#include <QFontDatabase>
#include <QSlider>
#include <QCheckBox>
#include <QApplication>
#include <QPainter>
#include <QFrame>
#include <QStackedLayout>
#include <QResizeEvent>
#include <QEvent>
#include <QKeyEvent>
#include <QPixmap>
#include <QTimer>
#include <QFile>
#include <QGraphicsBlurEffect>

Menu::Menu(QWidget *parent)
    : QWidget(parent),
      m_view(nullptr),
      m_scene(nullptr),
      m_timer(nullptr),
      m_overlayWidget(nullptr),
      m_mainMenuWidget(nullptr),
      m_optionsWidget(nullptr),
      m_pauseMenuWidget(nullptr),
      m_pauseOptionsWidget(nullptr),
      m_titleLabel(nullptr),
      m_startButton(nullptr),
      m_optionsButton(nullptr),
      m_leaderboardButton(nullptr),
      m_quitButton(nullptr),
      m_resumeButton(nullptr),
      m_pauseOptionsButton(nullptr),
      m_pauseQuitButton(nullptr),
      m_volumeSlider(nullptr),
      m_volumePercentLabel(nullptr),
      m_fullscreenCheck(nullptr),
      m_optionsBackButton(nullptr),
      m_pauseVolumeSlider(nullptr),
      m_pauseVolumePercentLabel(nullptr),
      m_pauseFullscreenCheck(nullptr),
      m_pauseOptionsBackButton(nullptr),
      m_selectorLabel(nullptr),
      m_selectedIndex(0),
      m_spawnTimer(0),
      m_usingPixelFont(false),
      m_currentPage(MainPage),
      m_leaderboard(nullptr),
      m_modalBackdrop(nullptr),
      m_isPauseMenuVisible(false)
{
    setFocusPolicy(Qt::StrongFocus);

    m_scene = new QGraphicsScene(this);
    m_scene->setSceneRect(-1000, -1000, 2000, 2000);
    m_scene->setBackgroundBrush(QBrush(QColor(10,10,30)));

    m_view = new QGraphicsView(m_scene, this);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setFrameStyle(QFrame::NoFrame);
    m_view->setRenderHint(QPainter::Antialiasing, false);
    m_view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    m_overlayWidget = new QWidget(this);
    m_overlayWidget->setStyleSheet("background: transparent;");

    setupUI();
    setupPauseMenuUI();

    // selector label (ship icon). try multiple paths for resource.
    m_selectorLabel = new QLabel(m_overlayWidget);
    m_selectorLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    QPixmap ship;
    const QStringList shipCandidates = {
        ":/resources/ship.png",
        ":/resources/spaceship.png",
        "./resources/ship.png",
        "./resources/spaceship.png"
    };
    for (const QString &p : shipCandidates) {
        // Try to load each path; stop when successful
        if (ship.load(p)) break;
    }

    if (!ship.isNull()) {
        QPixmap tinted(ship.size());
        tinted.fill(Qt::transparent);
        QPainter p(&tinted);
        p.drawPixmap(0,0, ship);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(tinted.rect(), Qt::white);
        p.end();
        m_selectorLabel->setPixmap(tinted.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        QPixmap tri(40,40);
        tri.fill(Qt::transparent);
        QPainter p(&tri);
        p.setRenderHint(QPainter::Antialiasing);
        QPoint pts[3] = { QPoint(4,20), QPoint(36,6), QPoint(36,34) };
        p.setBrush(QBrush(Qt::white));
        p.setPen(Qt::NoPen);
        p.drawPolygon(pts, 3);
        p.end();
        m_selectorLabel->setPixmap(tri);
    }
    m_selectorLabel->setFixedSize(40,40);
    m_selectorLabel->hide();

    // backdrop to dim menu when leaderboard visible
    m_modalBackdrop = new QWidget(this);
    m_modalBackdrop->setStyleSheet("background: rgba(0,0,0,160);");
    m_modalBackdrop->hide();

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &Menu::onUpdateScene);
    m_timer->start(16);

    for (int i = 0; i < 40; ++i)
        spawnSpaceObject();

    if (parent)
    {
        setGeometry(0, 0, parent->width(), parent->height());
        if (m_view) m_view->setGeometry(rect());
        if (m_overlayWidget) m_overlayWidget->setGeometry(rect());
        if (m_modalBackdrop) m_modalBackdrop->setGeometry(rect());

        parent->installEventFilter(this);
        m_view->installEventFilter(this);
    }

    layoutResponsive();

    // main/options items
    m_mainItems = { m_startButton, m_optionsButton, m_leaderboardButton, m_quitButton };
    m_optionsItems = { m_volumeSlider, m_fullscreenCheck, m_optionsBackButton };
    m_pauseItems = { m_resumeButton, m_pauseOptionsButton, m_pauseQuitButton };
    m_pauseOptionsItems = { m_pauseVolumeSlider, m_pauseFullscreenCheck, m_pauseOptionsBackButton };

    QTimer::singleShot(0, this, [this](){
        m_selectedIndex = 0;
        updateSelectorPosition();
        m_selectorLabel->show();
        ensureMenuFocus();
    });

    m_leaderboard = new Leaderboard(this);
    m_leaderboard->hide();
    m_leaderboard->setFocusPolicy(Qt::StrongFocus);
}

Menu::~Menu()
{
    for (SpaceObject *o : m_spaceObjects)
    {
        if (o->scene()) o->scene()->removeItem(o);
        delete o;
    }
    delete m_leaderboard;
    delete m_modalBackdrop;
}

void Menu::setupUI()
{
    QStackedLayout *stack = new QStackedLayout(m_overlayWidget);

    m_mainMenuWidget = new QWidget(m_overlayWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(m_mainMenuWidget);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setContentsMargins(40,40,40,40);
    mainLayout->setSpacing(24);

    m_titleLabel = new QLabel("INVADED\nSPACE", m_mainMenuWidget);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont("PressStart2P", 36, QFont::Bold);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet("QLabel { color: white; }");
    mainLayout->addWidget(m_titleLabel, 0, Qt::AlignHCenter);

    QWidget *btnBox = new QWidget(m_mainMenuWidget);
    QVBoxLayout *btnLayout = new QVBoxLayout(btnBox);
    btnLayout->setSpacing(12);
    btnLayout->setAlignment(Qt::AlignCenter);

    m_startButton = new QPushButton("Start Game", btnBox);
    m_optionsButton = new QPushButton("Options", btnBox);
    m_leaderboardButton = new QPushButton("Leaderboard", btnBox);
    m_quitButton = new QPushButton("Quit Game", btnBox);

    QString btnStyle =
        "QPushButton { color: white; background: rgba(30,30,50,200); border-radius:6px; padding: 10px; font-weight: bold; }"
        "QPushButton:hover { background: rgba(60,60,90,220); }";
    m_startButton->setStyleSheet(btnStyle);
    m_optionsButton->setStyleSheet(btnStyle);
    m_leaderboardButton->setStyleSheet(btnStyle);
    m_quitButton->setStyleSheet(btnStyle);

    m_startButton->setFixedWidth(260);
    m_optionsButton->setFixedWidth(260);
    m_leaderboardButton->setFixedWidth(260);
    m_quitButton->setFixedWidth(260);

    btnLayout->addWidget(m_startButton);
    btnLayout->addWidget(m_optionsButton);
    btnLayout->addWidget(m_leaderboardButton);
    btnLayout->addWidget(m_quitButton);
    mainLayout->addWidget(btnBox);

    m_optionsWidget = new QWidget(m_overlayWidget);
    QVBoxLayout *optLayout = new QVBoxLayout(m_optionsWidget);
    optLayout->setAlignment(Qt::AlignCenter);
    optLayout->setContentsMargins(40,40,40,40);
    optLayout->setSpacing(16);

    QLabel *volLabel = new QLabel("Sound volume (not implemented):", m_optionsWidget);
    volLabel->setStyleSheet("QLabel { color: white; }");
    optLayout->addWidget(volLabel, 0, Qt::AlignHCenter);

    QHBoxLayout *volRow = new QHBoxLayout();
    volRow->setSpacing(12);
    volRow->setAlignment(Qt::AlignCenter);

    m_volumeSlider = new QSlider(Qt::Horizontal, m_optionsWidget);
    m_volumeSlider->setRange(0,100);
    m_volumeSlider->setValue(80);
    m_volumeSlider->setFixedWidth(260);

    m_volumePercentLabel = new QLabel(QString("%1%").arg(m_volumeSlider->value()), m_optionsWidget);
    m_volumePercentLabel->setStyleSheet("QLabel { color: white; }");
    m_volumePercentLabel->setFixedWidth(48);
    m_volumePercentLabel->setAlignment(Qt::AlignCenter);

    volRow->addWidget(m_volumeSlider);
    volRow->addWidget(m_volumePercentLabel);
    optLayout->addLayout(volRow);

    m_fullscreenCheck = new QCheckBox("Fullscreen", m_optionsWidget);
    m_fullscreenCheck->setStyleSheet("QCheckBox { color: white; }");
    m_fullscreenCheck->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    optLayout->addWidget(m_fullscreenCheck, 0, Qt::AlignHCenter);

    m_optionsBackButton = new QPushButton("Back", m_optionsWidget);
    m_optionsBackButton->setStyleSheet(btnStyle);
    m_optionsBackButton->setFixedWidth(160);
    optLayout->addWidget(m_optionsBackButton, 0, Qt::AlignHCenter);

    stack->addWidget(m_mainMenuWidget);
    stack->addWidget(m_optionsWidget);
    stack->setCurrentWidget(m_mainMenuWidget);

    connect(m_startButton, &QPushButton::clicked, this, &Menu::onStartClicked);
    connect(m_optionsButton, &QPushButton::clicked, this, &Menu::onOptionsClicked);
    connect(m_leaderboardButton, &QPushButton::clicked, this, [this](){ showLeaderboard(!m_leaderboard->isVisible()); });
    connect(m_quitButton, &QPushButton::clicked, this, &Menu::onQuitClicked);
    connect(m_optionsBackButton, &QPushButton::clicked, this, &Menu::onBackFromOptions);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &Menu::onVolumeChanged);
    connect(m_fullscreenCheck, &QCheckBox::toggled, this, &Menu::onFullscreenChanged);
}

void Menu::setupPauseMenuUI()
{
    QStackedLayout *stack = qobject_cast<QStackedLayout*>(m_overlayWidget->layout());
    if (!stack) return;

    m_pauseMenuWidget = new QWidget(m_overlayWidget);
    QVBoxLayout *pauseLayout = new QVBoxLayout(m_pauseMenuWidget);
    pauseLayout->setAlignment(Qt::AlignCenter);
    pauseLayout->setContentsMargins(40,40,40,40);
    pauseLayout->setSpacing(24);

    QLabel *pauseTitle = new QLabel("PAUSED", m_pauseMenuWidget);
    pauseTitle->setAlignment(Qt::AlignCenter);
    QFont titleFont("PressStart2P", 36, QFont::Bold);
    pauseTitle->setFont(titleFont);
    pauseTitle->setStyleSheet("QLabel { color: white; }");
    pauseLayout->addWidget(pauseTitle, 0, Qt::AlignHCenter);

    QWidget *pauseBtnBox = new QWidget(m_pauseMenuWidget);
    QVBoxLayout *pauseBtnLayout = new QVBoxLayout(pauseBtnBox);
    pauseBtnLayout->setSpacing(12);
    pauseBtnLayout->setAlignment(Qt::AlignCenter);

    m_resumeButton = new QPushButton("Resume", pauseBtnBox);
    m_pauseOptionsButton = new QPushButton("Options", pauseBtnBox);
    m_pauseQuitButton = new QPushButton("Quit Game", pauseBtnBox);

    QString btnStyle =
        "QPushButton { color: white; background: rgba(30,30,50,200); border-radius:6px; padding: 10px; font-weight: bold; }"
        "QPushButton:hover { background: rgba(60,60,90,220); }";
    m_resumeButton->setStyleSheet(btnStyle);
    m_pauseOptionsButton->setStyleSheet(btnStyle);
    m_pauseQuitButton->setStyleSheet(btnStyle);

    m_resumeButton->setFixedWidth(260);
    m_pauseOptionsButton->setFixedWidth(260);
    m_pauseQuitButton->setFixedWidth(260);

    pauseBtnLayout->addWidget(m_resumeButton);
    pauseBtnLayout->addWidget(m_pauseOptionsButton);
    pauseBtnLayout->addWidget(m_pauseQuitButton);
    pauseLayout->addWidget(pauseBtnBox);

    m_pauseOptionsWidget = new QWidget(m_overlayWidget);
    QVBoxLayout *pauseOptLayout = new QVBoxLayout(m_pauseOptionsWidget);
    pauseOptLayout->setAlignment(Qt::AlignCenter);
    pauseOptLayout->setContentsMargins(40,40,40,40);
    pauseOptLayout->setSpacing(16);

    QLabel *pauseVolLabel = new QLabel("Sound volume (not implemented):", m_pauseOptionsWidget);
    pauseVolLabel->setStyleSheet("QLabel { color: white; }");
    pauseOptLayout->addWidget(pauseVolLabel, 0, Qt::AlignHCenter);

    QHBoxLayout *pauseVolRow = new QHBoxLayout();
    pauseVolRow->setSpacing(12);
    pauseVolRow->setAlignment(Qt::AlignCenter);

    m_pauseVolumeSlider = new QSlider(Qt::Horizontal, m_pauseOptionsWidget);
    m_pauseVolumeSlider->setRange(0,100);
    m_pauseVolumeSlider->setValue(80);
    m_pauseVolumeSlider->setFixedWidth(260);

    m_pauseVolumePercentLabel = new QLabel(QString("%1%").arg(m_pauseVolumeSlider->value()), m_pauseOptionsWidget);
    m_pauseVolumePercentLabel->setStyleSheet("QLabel { color: white; }");
    m_pauseVolumePercentLabel->setFixedWidth(48);
    m_pauseVolumePercentLabel->setAlignment(Qt::AlignCenter);

    pauseVolRow->addWidget(m_pauseVolumeSlider);
    pauseVolRow->addWidget(m_pauseVolumePercentLabel);
    pauseOptLayout->addLayout(pauseVolRow);

    m_pauseFullscreenCheck = new QCheckBox("Fullscreen", m_pauseOptionsWidget);
    m_pauseFullscreenCheck->setStyleSheet("QCheckBox { color: white; }");
    m_pauseFullscreenCheck->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    pauseOptLayout->addWidget(m_pauseFullscreenCheck, 0, Qt::AlignHCenter);

    m_pauseOptionsBackButton = new QPushButton("Back", m_pauseOptionsWidget);
    m_pauseOptionsBackButton->setStyleSheet(btnStyle);
    m_pauseOptionsBackButton->setFixedWidth(160);
    pauseOptLayout->addWidget(m_pauseOptionsBackButton, 0, Qt::AlignHCenter);

    stack->addWidget(m_pauseMenuWidget);
    stack->addWidget(m_pauseOptionsWidget);

    connect(m_resumeButton, &QPushButton::clicked, this, &Menu::onResumeClicked);
    connect(m_pauseOptionsButton, &QPushButton::clicked, this, &Menu::onPauseOptionsClicked);
    connect(m_pauseQuitButton, &QPushButton::clicked, this, &Menu::onPauseQuitClicked);
    connect(m_pauseOptionsBackButton, &QPushButton::clicked, this, &Menu::onBackFromPauseOptions);
    connect(m_pauseVolumeSlider, &QSlider::valueChanged, this, &Menu::onVolumeChanged);
    connect(m_pauseFullscreenCheck, &QCheckBox::toggled, this, &Menu::onFullscreenChanged);
}

void Menu::showPauseMenu(bool show)
{
    m_isPauseMenuVisible = show;

    if (show) {
        m_currentPage = PauseMenuPage;
        QStackedLayout *stack = qobject_cast<QStackedLayout*>(m_overlayWidget->layout());
        if (stack) stack->setCurrentWidget(m_pauseMenuWidget);
        m_selectedIndex = 0;
        
        // Show as modal overlay with blur effect on background
        if (m_view) {
            m_view->setGraphicsEffect(new QGraphicsBlurEffect());
            if (auto blurEffect = qobject_cast<QGraphicsBlurEffect*>(m_view->graphicsEffect())) {
                blurEffect->setBlurRadius(10);
            }
        }
        
        // Set overlay with semi-transparent dark background
        m_overlayWidget->setStyleSheet("background: rgba(0, 0, 0, 80);");
        m_overlayWidget->show();
        m_overlayWidget->raise();
        
        this->setStyleSheet("background: transparent;");
        this->show();
        this->raise();
        ensureMenuFocus();
        updateSelectorPosition();
    } else {
        m_isPauseMenuVisible = false;
        m_overlayWidget->setStyleSheet("background: transparent;");
        if (m_view) {
            m_view->setGraphicsEffect(nullptr);  // Remove blur effect
        }
        this->hide();
        emit resumeGameRequested();
    }
}

bool Menu::isPauseMenuVisible() const
{
    return m_isPauseMenuVisible;
}

void Menu::showLeaderboard(bool show)
{
    if (!m_leaderboard || !m_modalBackdrop) return;

    if (show) {
        QSize s = size();
        int lw = qBound(400, s.width() * 60 / 100, s.width() - 40);
        int lh = qBound(300, s.height() * 70 / 100, s.height() - 40);
        m_modalBackdrop->setGeometry(rect());
        m_modalBackdrop->show();
        m_leaderboard->setGeometry((s.width()-lw)/2, (s.height()-lh)/2, lw, lh);
        m_leaderboard->show();
        m_leaderboard->raise();
        m_leaderboard->setFocus();

        if (m_overlayWidget && m_overlayWidget->layout())
            m_overlayWidget->layout()->activate();

        QTimer::singleShot(0, this, [this, lw, lh]() {
            // reposition selector (if needed) and ensure leaderboard centered
            updateSelectorPosition();
            if (m_leaderboard && m_leaderboard->isVisible()) {
                QSize s2 = size();
                int lw2 = qBound(400, s2.width() * 60 / 100, s2.width() - 40);
                int lh2 = qBound(300, s2.height() * 70 / 100, s2.height() - 40);
                m_leaderboard->setGeometry((s2.width()-lw2)/2, (s2.height()-lh2)/2, lw2, lh2);
            }
        });
    } else {
        m_leaderboard->hide();
        m_modalBackdrop->hide();
        ensureMenuFocus();
    }
}

void Menu::onOptionsClicked()
{
    m_currentPage = OptionsPage;
    QStackedLayout *stack = qobject_cast<QStackedLayout*>(m_overlayWidget->layout());
    if (stack) stack->setCurrentWidget(m_optionsWidget);
    m_selectedIndex = 0;
    updateSelectorPosition();
}

void Menu::onResumeClicked()
{
    showPauseMenu(false);
}

void Menu::onPauseOptionsClicked()
{
    m_currentPage = PauseOptionsPage;
    QStackedLayout *stack = qobject_cast<QStackedLayout*>(m_overlayWidget->layout());
    if (stack) stack->setCurrentWidget(m_pauseOptionsWidget);
    m_selectedIndex = 0;
    updateSelectorPosition();
}

void Menu::onBackFromPauseOptions()
{
    m_currentPage = PauseMenuPage;
    QStackedLayout *stack = qobject_cast<QStackedLayout*>(m_overlayWidget->layout());
    if (stack) stack->setCurrentWidget(m_pauseMenuWidget);
    m_selectedIndex = 0;
    updateSelectorPosition();
}

void Menu::onPauseQuitClicked()
{
    showPauseMenu(false);
    qApp->quit();
}

void Menu::keyPressEvent(QKeyEvent *event)
{
    if (m_leaderboard && m_leaderboard->isVisible()) {
        // forward navigation to leaderboard
        int key = event->key();
        if (key == Qt::Key_W || key == Qt::Key_Up) { m_leaderboard->selectPrevious(); event->accept(); return; }
        if (key == Qt::Key_S || key == Qt::Key_Down) { m_leaderboard->selectNext(); event->accept(); return; }
        if (key == Qt::Key_Return || key == Qt::Key_Enter) { m_leaderboard->activateSelected(); event->accept(); return; }
        if (key == Qt::Key_Escape) { showLeaderboard(false); event->accept(); return; }
    }

    int key = event->key();

    // Allow Escape to resume game from pause menu
    if (key == Qt::Key_Escape && m_isPauseMenuVisible) {
        showPauseMenu(false);
        event->accept();
        return;
    }

    if (key == Qt::Key_W || key == Qt::Key_Up) {
        moveSelection(-1);
        event->accept();
        return;
    }
    if (key == Qt::Key_S || key == Qt::Key_Down) {
        moveSelection(+1);
        event->accept();
        return;
    }

    if (key == Qt::Key_A || key == Qt::Key_Left) {
        QVector<QWidget*> *items = nullptr;
        if (m_currentPage == OptionsPage) {
            items = &m_optionsItems;
        } else if (m_currentPage == PauseOptionsPage) {
            items = &m_pauseOptionsItems;
        }

        if (items && m_selectedIndex >=0 && m_selectedIndex < items->size()) {
            QWidget *w = (*items)[m_selectedIndex];
            if (auto slider = qobject_cast<QSlider*>(w)) {
                int v = slider->value();
                slider->setValue(qMax(0, v - 5));
            } else if (auto cb = qobject_cast<QCheckBox*>(w)) {
                cb->toggle();
            }
        }
        event->accept();
        return;
    }
    if (key == Qt::Key_D || key == Qt::Key_Right) {
        QVector<QWidget*> *items = nullptr;
        if (m_currentPage == OptionsPage) {
            items = &m_optionsItems;
        } else if (m_currentPage == PauseOptionsPage) {
            items = &m_pauseOptionsItems;
        }

        if (items && m_selectedIndex >=0 && m_selectedIndex < items->size()) {
            QWidget *w = (*items)[m_selectedIndex];
            if (auto slider = qobject_cast<QSlider*>(w)) {
                int v = slider->value();
                slider->setValue(qMin(100, v + 5));
            } else if (auto cb = qobject_cast<QCheckBox*>(w)) {
                cb->toggle();
            }
        }
        event->accept();
        return;
    }

    if (key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_Space) {
        activateSelected();
        event->accept();
        return;
    }

    QWidget::keyPressEvent(event);
}

bool Menu::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Resize && watched == parentWidget())
    {
        QResizeEvent *re = static_cast<QResizeEvent*>(event);
        setGeometry(0, 0, re->size().width(), re->size().height());
        if (m_view) m_view->setGeometry(rect());
        if (m_overlayWidget) m_overlayWidget->setGeometry(rect());
        if (m_modalBackdrop) m_modalBackdrop->setGeometry(rect());
        layoutResponsive();

        // Defer selector and leaderboard repositioning until layouts update (prevents stale coords)
        if (m_overlayWidget && m_overlayWidget->layout())
            m_overlayWidget->layout()->activate();

        QTimer::singleShot(0, this, [this]() {
            updateSelectorPosition();
            if (m_leaderboard && m_leaderboard->isVisible()) {
                QSize s = size();
                int lw = qBound(400, s.width() * 60 / 100, s.width() - 40);
                int lh = qBound(300, s.height() * 70 / 100, s.height() - 40);
                m_leaderboard->setGeometry((s.width()-lw)/2, (s.height()-lh)/2, lw, lh);
            }
        });

        return false;
    }

    if (event->type() == QEvent::Wheel)
    {
        if (isVisible()) return true;
    }

    return QWidget::eventFilter(watched, event);
}

void Menu::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_view) m_view->setGeometry(rect());
    if (m_overlayWidget) m_overlayWidget->setGeometry(rect());
    layoutResponsive();

    if (m_overlayWidget && m_overlayWidget->layout())
        m_overlayWidget->layout()->activate();

    // Defer selector update so child widgets have correct geometry
    QTimer::singleShot(0, this, [this]() {
        updateSelectorPosition();
    });
}

void Menu::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    ensureMenuFocus();
    if (parentWidget()) parentWidget()->installEventFilter(this);
}

void Menu::ensureMenuFocus()
{
    setFocus(Qt::ActiveWindowFocusReason);
}

void Menu::layoutResponsive()
{
    int h = height();
    int w = width();
    if (h <= 0 || w <= 0) return;

    int titleSize = qBound(18, h / 12, 72);
    QFont tf = m_titleLabel->font();
    tf.setPointSize(titleSize);
    m_titleLabel->setFont(tf);

    int bw = qBound(160, w / 4, 420);
    m_startButton->setFixedWidth(bw);
    m_optionsButton->setFixedWidth(bw);
    m_leaderboardButton->setFixedWidth(bw);
    m_quitButton->setFixedWidth(bw);
    m_resumeButton->setFixedWidth(bw);
    m_pauseOptionsButton->setFixedWidth(bw);
    m_pauseQuitButton->setFixedWidth(bw);

    if (m_volumeSlider)
    {
        int sliderW = qBound(140, w / 5, 360);
        m_volumeSlider->setFixedWidth(sliderW);
    }

    if (m_pauseVolumeSlider)
    {
        int sliderW = qBound(140, w / 5, 360);
        m_pauseVolumeSlider->setFixedWidth(sliderW);
    }

    if (m_optionsBackButton)
        m_optionsBackButton->setFixedWidth(qBound(120, bw/2, 300));

    if (m_pauseOptionsBackButton)
        m_pauseOptionsBackButton->setFixedWidth(qBound(120, bw/2, 300));
}

void Menu::onQuitClicked()
{
    qApp->quit();
}

void Menu::onBackFromOptions()
{
    m_currentPage = MainPage;
    if (m_overlayWidget) {
        QStackedLayout *stack = qobject_cast<QStackedLayout*>(m_overlayWidget->layout());
        if (stack) stack->setCurrentWidget(m_mainMenuWidget);
    }
    m_selectedIndex = 0;
    updateSelectorPosition();
}

void Menu::onVolumeChanged(int value)
{
    if (m_volumePercentLabel)
        m_volumePercentLabel->setText(QString("%1%").arg(value));
    if (m_pauseVolumePercentLabel)
        m_pauseVolumePercentLabel->setText(QString("%1%").arg(value));
    emit volumeChanged(value);
}

void Menu::onFullscreenChanged(bool checked)
{
    emit fullscreenToggled(checked);
    ensureMenuFocus();
}

void Menu::onUpdateScene()
{
    QList<SpaceObject*> toRemove;
    for (SpaceObject *o : m_spaceObjects)
    {
        if (!o) continue;
        o->update();
        // remove when lifetime property expires (some SpaceObject implementations use this)
        if (o->property("lifetime").isValid() && o->property("lifetime").toReal() <= 0)
            toRemove.append(o);
    }

    for (SpaceObject *r : toRemove)
    {
        m_spaceObjects.removeOne(r);
        if (r->scene()) r->scene()->removeItem(r);
        delete r;
    }

    m_spawnTimer++;
    if (m_spawnTimer >= 120) // ~2s at 60FPS
    {
        m_spawnTimer = 0;
        spawnSpaceObject();
    }
}

void Menu::onStartClicked()
{
    // Start game and hide menu overlay
    emit startGameRequested();
    hide();
}

void Menu::spawnSpaceObject()
{
    QRandomGenerator *rng = QRandomGenerator::global();

    qreal x = rng->bounded(-1000, 1000);
    qreal y = rng->bounded(-1000, 1000);

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
    if (m_scene) m_scene->addItem(obj);
    m_spaceObjects.append(obj);
}

void Menu::activateSelected()
{
    QVector<QWidget*> *list = nullptr;

    if (m_currentPage == MainPage) {
        list = &m_mainItems;
    } else if (m_currentPage == OptionsPage) {
        list = &m_optionsItems;
    } else if (m_currentPage == PauseMenuPage) {
        list = &m_pauseItems;
    } else if (m_currentPage == PauseOptionsPage) {
        list = &m_pauseOptionsItems;
    }

    if (!list || list->isEmpty()) return;

    int idx = qBound(0, m_selectedIndex, list->size() - 1);
    QWidget *w = (*list)[idx];
    if (!w) return;

    if (auto pb = qobject_cast<QPushButton*>(w)) {
        // Handle leaderboard button specially so keyboard activation uses showLeaderboard
        if (pb == m_leaderboardButton) {
            showLeaderboard(true);
        } else {
            pb->click();
        }
    }
    else if (auto slider = qobject_cast<QSlider*>(w)) {
        Q_UNUSED(slider);
        // no action on Enter for slider
    }
    else if (auto cb = qobject_cast<QCheckBox*>(w)) {
        cb->toggle();
    }
}

void Menu::loadPixelFontIfAvailable()
{
    const QStringList candidates = {
        "./resources/SpaceFont.ttf",
        ":/resources/SpaceFont.ttf",
    };

    for (const QString &path : candidates)
    {
        int id = QFontDatabase::addApplicationFont(path);
        if (id != -1)
        {
            QString family = QFontDatabase::applicationFontFamilies(id).value(0);
            if (!family.isEmpty())
            {
                QFont f(family);
                f.setBold(true);
                m_titleLabel->setFont(f);
                m_usingPixelFont = true;
                m_titleLabel->setStyleSheet("QLabel { color: white; }");
                return;
            }
        }
    }
}

void Menu::updateSelectorPosition()
{
    QVector<QWidget*> *list = nullptr;

    if (m_currentPage == MainPage) {
        list = &m_mainItems;
    } else if (m_currentPage == OptionsPage) {
        list = &m_optionsItems;
    } else if (m_currentPage == PauseMenuPage) {
        list = &m_pauseItems;
    } else if (m_currentPage == PauseOptionsPage) {
        list = &m_pauseOptionsItems;
    }

    if (!list || list->isEmpty() || !m_selectorLabel || !m_overlayWidget) {
        if (m_selectorLabel) m_selectorLabel->hide();
        return;
    }

    int idx = qBound(0, m_selectedIndex, list->size() - 1);
    QWidget *w = (*list)[idx];
    if (!w) {
        m_selectorLabel->hide();
        return;
    }

    // Ensure layout geometry is up to date
    w->updateGeometry();
    m_overlayWidget->updateGeometry();

    QPoint local = w->mapTo(m_overlayWidget, QPoint(0, 0));
    int y = local.y() + w->height() / 2 - m_selectorLabel->height() / 2;
    int x = local.x() - m_selectorLabel->width() - 8;
    if (x < 8) x = 8;
    m_selectorLabel->move(x, y);
    m_selectorLabel->raise();
    m_selectorLabel->show();

    // Visual highlight for buttons only
    QString baseBtn =
        "QPushButton { color: white; background: rgba(30,30,50,200); border-radius:6px; padding: 10px; font-weight: bold; }"
        "QPushButton:hover { background: rgba(60,60,90,220); }";
    QString highlight = "outline: 2px solid rgba(200,200,255,0.18);";

    auto highlightButtons = [&](QVector<QWidget*> &items) {
        for (int i = 0; i < items.size(); ++i) {
            QWidget *u = items[i];
            if (!u) continue;
            if (auto pb = qobject_cast<QPushButton*>(u)) {
                pb->setStyleSheet((i == idx) ? baseBtn + " " + highlight : baseBtn);
            }
        }
    };

    if (m_currentPage == MainPage) {
        highlightButtons(m_mainItems);
    } else if (m_currentPage == OptionsPage) {
        highlightButtons(m_optionsItems);
    } else if (m_currentPage == PauseMenuPage) {
        highlightButtons(m_pauseItems);
    } else if (m_currentPage == PauseOptionsPage) {
        highlightButtons(m_pauseOptionsItems);
    }
}

void Menu::moveSelection(int delta)
{
    QVector<QWidget*> *list = nullptr;

    if (m_currentPage == MainPage) {
        list = &m_mainItems;
    } else if (m_currentPage == OptionsPage) {
        list = &m_optionsItems;
    } else if (m_currentPage == PauseMenuPage) {
        list = &m_pauseItems;
    } else if (m_currentPage == PauseOptionsPage) {
        list = &m_pauseOptionsItems;
    }

    if (!list || list->isEmpty()) return;

    m_selectedIndex += delta;
    if (m_selectedIndex < 0) m_selectedIndex = list->size() - 1;
    if (m_selectedIndex >= list->size()) m_selectedIndex = 0;

    updateSelectorPosition();
}
