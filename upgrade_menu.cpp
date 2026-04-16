#include "upgrade_menu.h"
#include "SoundManager.h"
#include <QKeyEvent>
#include <QApplication>
#include <QEvent>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QVBoxLayout>
#include <QParallelAnimationGroup>
#include <QShowEvent>
#include <cmath>

UpgradeMenu::UpgradeMenu(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("LEVEL UP!");
    setWindowFlags(Qt::FramelessWindowHint);
    setParent(parent);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);
    mainLayout->addStretch(1);
    
    title = new QLabel("LEVEL UP!", this);
    title->setAlignment(Qt::AlignCenter);
    title->setWordWrap(true);
    
    QGraphicsDropShadowEffect* titleShadow = new QGraphicsDropShadowEffect(this);
    titleShadow->setBlurRadius(15);
    titleShadow->setColor(QColor(100, 150, 255, 180));
    titleShadow->setOffset(0, 0);
    title->setGraphicsEffect(titleShadow);
    
    mainLayout->addWidget(title, 0, Qt::AlignCenter);

    QWidget *buttonContainer = new QWidget(this);
    QVBoxLayout *btnContainerLayout = new QVBoxLayout(buttonContainer);
    btnContainerLayout->setContentsMargins(0, 0, 0, 0);
    btnContainerLayout->setSpacing(15);

    btnSpeed = new QPushButton("Speed (+0.4)", this);
    btnDamage = new QPushButton("Damage (+1)", this);
    btnHealth = new QPushButton("Max Health (+2)", this);

    btnContainerLayout->addWidget(btnSpeed, 0, Qt::AlignCenter);
    btnContainerLayout->addWidget(btnDamage, 0, Qt::AlignCenter);
    btnContainerLayout->addWidget(btnHealth, 0, Qt::AlignCenter);

    mainLayout->addWidget(buttonContainer, 0, Qt::AlignCenter);
    mainLayout->addStretch(1);

    connect(btnSpeed, &QPushButton::clicked, [this]() { 
        SoundManager::instance()->playSound(SoundManager::UpgradeSelected);
        emit upgradeSelected(0); 
        accept(); 
    });
    connect(btnDamage, &QPushButton::clicked, [this]() { 
        SoundManager::instance()->playSound(SoundManager::UpgradeSelected);
        emit upgradeSelected(1); 
        accept(); 
    });
    connect(btnHealth, &QPushButton::clicked, [this]() { 
        SoundManager::instance()->playSound(SoundManager::UpgradeSelected);
        emit upgradeSelected(2); 
        accept(); 
    });
    
    this->setStyleSheet(R"(
    QDialog {
        background-color: rgba(18, 18, 18, 240);
        border-radius: 12px;
    }

    QLabel {
        color: white;
        font-weight: bold;
    }

    QPushButton {
        color: white;
        background-color: rgba(40, 40, 70, 220);
        border-radius: 8px;
        padding: 12px;
        font-weight: bold;
        border: none;
        min-width: 200px;
    }

    QPushButton:hover {
        background-color: rgba(70, 70, 120, 240);
    }

    QPushButton:focus {
        border: 2px solid rgba(200, 200, 255, 200);
    }
    )");

    if (parent) {
        parent->installEventFilter(this);
        updateCenteredGeometry();
    }

    layoutResponsive();
    setupTitleAnimation();
    setupEntranceAnimation();
    
    setWindowOpacity(0.0);
}

UpgradeMenu::~UpgradeMenu()
{
    if (m_animationTimer) {
        m_animationTimer->stop();
        delete m_animationTimer;
    }
    if (m_entranceAnimation) {
        m_entranceAnimation->stop();
        delete m_entranceAnimation;
    }
}

void UpgradeMenu::setupTitleAnimation()
{
    if (!title) return;

    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, &UpgradeMenu::onAnimationTick);
    m_animationTimer->start(50);
}

void UpgradeMenu::setupEntranceAnimation()
{
    m_entranceAnimation = new QParallelAnimationGroup(this);

    QPropertyAnimation* opacityAnim = new QPropertyAnimation(this, "windowOpacity");
    opacityAnim->setDuration(500);
    opacityAnim->setStartValue(0.0);
    opacityAnim->setEndValue(1.0);
    opacityAnim->setEasingCurve(QEasingCurve::InOutQuad);

    QPropertyAnimation* geometryAnim = new QPropertyAnimation(this, "geometry");
    geometryAnim->setDuration(500);
    QRect startGeom = geometry();
    startGeom.adjust(0, -50, 0, -50);
    geometryAnim->setStartValue(startGeom);
    geometryAnim->setEndValue(geometry());
    geometryAnim->setEasingCurve(QEasingCurve::OutBack);

    m_entranceAnimation->addAnimation(opacityAnim);
    m_entranceAnimation->addAnimation(geometryAnim);
}

void UpgradeMenu::onAnimationTick()
{
    if (!title) return;

    static qreal time = 0.0;
    time += 0.05;

    qreal pulse = 12.0 + 8.0 * std::sin(time);
    m_glowIntensity = pulse;

    if (auto shadowEffect = qobject_cast<QGraphicsDropShadowEffect*>(title->graphicsEffect())) {
        shadowEffect->setBlurRadius(pulse);
        
        int colorAlpha = static_cast<int>(150 + 80 * std::sin(time));
        colorAlpha = qBound(70, colorAlpha, 230);
        
        QColor glowColor(100, 150, 255, colorAlpha);
        shadowEffect->setColor(glowColor);
    }
}

bool UpgradeMenu::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::Resize && watched == parentWidget())
    {
        QResizeEvent* re = static_cast<QResizeEvent*>(event);
        updateCenteredGeometry();
        layoutResponsive();
        return false;
    }

    return QDialog::eventFilter(watched, event);
}

void UpgradeMenu::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);
    layoutResponsive();
}

void UpgradeMenu::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    
    SoundManager::instance()->playSound(SoundManager::UpgradeMenuAppear);
    
    if (m_entranceAnimation) {
        m_entranceAnimation->start();
    }
}

void UpgradeMenu::layoutResponsive()
{
    if (!title || !btnSpeed || !btnDamage || !btnHealth)
        return;

    int w = width();
    int h = height();

    if (w <= 0 || h <= 0) return;

    bool fullscreen = parentWidget() && parentWidget()->isFullScreen();

    int titleSize = fullscreen
        ? qBound(28, h / 6, 56)
        : qBound(16, h / 10, 32);

    int btnFontSize = fullscreen
        ? qBound(12, h / 14, 24)
        : qBound(9, h / 18, 16);

    int btnHeight = fullscreen
        ? qBound(50, h / 10, 90)
        : qBound(36, h / 11, 60);

    int btnWidth = qBound(180, w / 2 - 20, fullscreen ? 450 : 300);

    QFont titleFont = title->font();
    titleFont.setPointSize(titleSize);
    titleFont.setBold(true);
    title->setFont(titleFont);
    
    title->setMinimumHeight(titleSize * 2 + 20);
    title->setMaximumHeight(h / 3);

    QFont btnFont;
    btnFont.setPointSize(btnFontSize);
    btnFont.setBold(true);

    QList<QPushButton*> buttons = { btnSpeed, btnDamage, btnHealth };

    for (QPushButton* b : buttons) {
        b->setFont(btnFont);
        b->setMinimumSize(btnWidth, btnHeight);
        b->setMaximumHeight(btnHeight + 10);
    }

    if (layout()) {
        layout()->setSpacing(h / 30);
        layout()->update();
    }
}

void UpgradeMenu::updateCenteredGeometry()
{
    if (!parentWidget()) return;

    QSize parentSize = parentWidget()->size();

    int dialogWidth = qBound(350, parentSize.width() * 50 / 100, 700);
    int dialogHeight = qBound(300, parentSize.height() * 55 / 100, 550);

    int x = (parentSize.width() - dialogWidth) / 2;
    int y = (parentSize.height() - dialogHeight) / 2;

    setGeometry(x, y, dialogWidth, dialogHeight);
    setMinimumSize(dialogWidth, dialogHeight);
}

void UpgradeMenu::navigateWithJoystick(double x, double y, bool tir, bool ultimate) 
{
    if ((y > 0.7 || y < -0.7) && joystickAuNeutre) {
        this->focusNextChild();
        joystickAuNeutre = false;
    }

    if (y > -0.2 && y < 0.2) {
        joystickAuNeutre = true;
    }

    if (tir) {
        QWidget* boutonActuel = this->focusWidget();

        if (boutonActuel) {
            QKeyEvent pressSpace(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
            QApplication::sendEvent(boutonActuel, &pressSpace);

            QKeyEvent releaseSpace(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier);
            QApplication::sendEvent(boutonActuel, &releaseSpace);
        }
    }
}
