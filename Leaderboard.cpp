#include "leaderboard.h"
#include "JsonParser.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QWheelEvent>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QFile>
#include <QPushButton>
#include <QPainter>

Leaderboard::Leaderboard(QWidget* parent) : QWidget(parent), m_scroll(nullptr),
    m_container(nullptr), m_listLayout(nullptr), m_selectedIndex(-1),
    m_buttonContainer(nullptr), m_buttonLayout(nullptr), 
    m_restartButton(nullptr), m_quitButton(nullptr), m_isEndgameMode(false)
{
    setStyleSheet("background-color: rgba(15, 15, 15, 240); border: 2px solid white; border-radius: 10px;");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12,12,12,12);
    mainLayout->setSpacing(8);

    QLabel* title = new QLabel("LEADERBOARD", this);
    title->setStyleSheet("color: white; font-weight: bold;");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);
    m_scroll->setStyleSheet("background: transparent; border: none;");

    m_container = new QWidget(m_scroll);
    m_listLayout = new QVBoxLayout(m_container);
    m_listLayout->setContentsMargins(6,6,6,6);
    m_listLayout->setSpacing(10);

    m_scroll->setWidget(m_container);
    mainLayout->addWidget(m_scroll, 1);

    m_scroll->viewport()->installEventFilter(this);

    // Endgame button
    m_buttonContainer = new QWidget(this);
    m_buttonLayout = new QVBoxLayout(m_buttonContainer);
    m_buttonLayout->setContentsMargins(0,8,0,0);
    m_buttonLayout->setSpacing(12);
    m_buttonLayout->setAlignment(Qt::AlignCenter);

    QString btnStyle =
        "QPushButton { color: white; background: rgba(30,30,50,200); border-radius:6px; padding: 10px; font-weight: bold; }"
        "QPushButton:hover { background: rgba(60,60,90,220); }";

    m_restartButton = new QPushButton("Restart Game", m_buttonContainer);
    m_restartButton->setStyleSheet(btnStyle);
    m_quitButton = new QPushButton("Quit Game", m_buttonContainer);
    m_quitButton->setStyleSheet(btnStyle);

    m_buttonLayout->addWidget(m_restartButton);
    m_buttonLayout->addWidget(m_quitButton);

    m_buttonContainer->hide();
    mainLayout->addWidget(m_buttonContainer);

    connect(m_restartButton, &QPushButton::clicked, this, &Leaderboard::restartRequested);
    connect(m_quitButton, &QPushButton::clicked, this, &Leaderboard::quitRequested);

    refresh();
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void Leaderboard::refresh()
{
    qDeleteAll(m_entries);
    m_entries.clear();
    QLayoutItem *child;
    while ((child = m_listLayout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }

    QList<QJsonObject> players = JsonParser::readAllSorted();
    for (const QJsonObject& obj : players) {
        QFrame* f = createEntryWidget(obj);
        m_entries.append(f);
        m_listLayout->addWidget(f);
    }
    m_listLayout->addStretch();
    m_selectedIndex = m_entries.isEmpty() ? -1 : 0;
    updateSelectionUI();
    updateFontSizes();
}

QFrame* Leaderboard::createEntryWidget(const QJsonObject& obj) {
    QFrame* frame = new QFrame();
    frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    frame->setStyleSheet(
        "QFrame { border: 2px solid white; background-color: #121212; border-radius: 5px; }"
        "QLabel { border: none; color: white; background: none; }"
    );

    auto* layout = new QHBoxLayout(frame);
    layout->setContentsMargins(8,8,8,8);
    layout->setSpacing(12);

    QLabel* iconLabel = new QLabel(frame);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("border: 1px solid #444; background: #050505;");

    QPixmap playerImg;
    playerImg.load("./resources/spaceship.png");

    if (!playerImg.isNull()) {
        // Parse the color from JSON
        QString colorHex = obj["color"].toString();
        QColor shipColor;
        
        // Parse hex color
        if (colorHex.startsWith("#") && colorHex.length() >= 7) {
            colorHex = colorHex.mid(1);
            
            if (colorHex.length() == 6) {
                colorHex += "B4";
            }
            
            if (colorHex.length() == 8) {
                bool ok;
                unsigned int rgba = colorHex.toUInt(&ok, 16);
                if (ok) {
                    int r = (rgba >> 24) & 0xFF;
                    int g = (rgba >> 16) & 0xFF;
                    int b = (rgba >> 8) & 0xFF;
                    int a = rgba & 0xFF;
                    shipColor = QColor(r, g, b, a);
                }
            }
        }
        
        // Apply color to ship sprite
        QPixmap coloredShip = playerImg;
        if (shipColor.isValid() && shipColor.alpha() > 0) {
            QPixmap tinted(playerImg.size());
            tinted.fill(Qt::transparent);
            
            QPainter painter(&tinted);
            painter.drawPixmap(0, 0, playerImg);
            
            painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
            QColor reducedColor = shipColor;
            reducedColor.setAlpha(static_cast<int>(shipColor.alpha() * 0.8));
            painter.fillRect(tinted.rect(), reducedColor);
            painter.end();
            
            coloredShip = tinted;
        }
        
        iconLabel->setPixmap(coloredShip.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        iconLabel->setText("?");
        iconLabel->setStyleSheet("color: white; font-weight: bold; border: 1px solid #444;");
    }
    iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QLabel* stats = new QLabel(QString("<b>%1</b><br/>HP:%2 | LVL:%3 | ATK:%4")
        .arg(obj["name"].toString()).arg(obj["hp"].toInt()).arg(obj["lvl"].toInt()).arg(obj["atk"].toInt()), frame);
    stats->setTextFormat(Qt::RichText);
    stats->setWordWrap(true);

    QLabel* score = new QLabel(QString("<b style='color:#FF5252;'>%1</b>").arg(obj["score"].toInt()), frame);
    score->setAlignment(Qt::AlignCenter);
    score->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    layout->addWidget(iconLabel);
    layout->addWidget(stats, 1);
    layout->addStretch();
    layout->addWidget(score);
    return frame;
}

void Leaderboard::updateFontSizes()
{
    int w = width();
    if (w <= 0) return;

    int titleSize = qBound(14, w / 20, 28);
    QFont titleFont;
    titleFont.setPointSize(titleSize);
    titleFont.setBold(true);

    QLabel* title = findChild<QLabel*>();
    if (title) title->setFont(titleFont);

    int statsFontSize = qBound(9, w / 35, 14);

    for (QFrame* frame : m_entries) {
        auto labels = frame->findChildren<QLabel*>();
        for (int i = 0; i < labels.size(); ++i) {
            QLabel* lbl = labels[i];
            if (i == 0) continue;
            
            QFont f = lbl->font();
            f.setPointSize(statsFontSize);
            lbl->setFont(f);
        }
    }

    int btnFontSize = qBound(10, w / 40, 12);
    if (m_restartButton) {
        QFont f = m_restartButton->font();
        f.setPointSize(btnFontSize);
        m_restartButton->setFont(f);
        int btnWidth = qBound(140, w / 3, 280);
        m_restartButton->setFixedWidth(btnWidth);
    }
    if (m_quitButton) {
        QFont f = m_quitButton->font();
        f.setPointSize(btnFontSize);
        m_quitButton->setFont(f);
        int btnWidth = qBound(140, w / 3, 280);
        m_quitButton->setFixedWidth(btnWidth);
    }
}

void Leaderboard::updateSelectionUI()
{
    if (m_isEndgameMode) {
        if (m_selectedIndex == 0 && m_restartButton) {
            m_restartButton->setStyleSheet(
                "QPushButton { color: white; background: rgba(30,30,50,200); border-radius:6px; padding: 10px; font-weight: bold; outline: 2px solid rgba(200,200,255,0.18); }"
                "QPushButton:hover { background: rgba(60,60,90,220); }"
            );
            m_quitButton->setStyleSheet(
                "QPushButton { color: white; background: rgba(30,30,50,200); border-radius:6px; padding: 10px; font-weight: bold; }"
                "QPushButton:hover { background: rgba(60,60,90,220); }"
            );
        } else if (m_selectedIndex == 1 && m_quitButton) {
            m_restartButton->setStyleSheet(
                "QPushButton { color: white; background: rgba(30,30,50,200); border-radius:6px; padding: 10px; font-weight: bold; }"
                "QPushButton:hover { background: rgba(60,60,90,220); }"
            );
            m_quitButton->setStyleSheet(
                "QPushButton { color: white; background: rgba(30,30,50,200); border-radius:6px; padding: 10px; font-weight: bold; outline: 2px solid rgba(200,200,255,0.18); }"
                "QPushButton:hover { background: rgba(60,60,90,220); }"
            );
        }
        return;
    }

    // Normal leaderboard mode
    for (int i = 0; i < m_entries.size(); ++i) {
        QFrame* f = m_entries[i];
        if (!f) continue;
        if (i == m_selectedIndex) {
            f->setStyleSheet(
                "QFrame { border: 2px solid #8fb3ff; background-color: #1a1a2a; border-radius: 5px; }"
                "QLabel { color: white; }"
            );
        } else {
            f->setStyleSheet(
                "QFrame { border: 2px solid white; background-color: #121212; border-radius: 5px; }"
                "QLabel { color: white; }"
            );
        }
    }
    if (m_selectedIndex >= 0 && m_selectedIndex < m_entries.size()) {
        m_scroll->ensureWidgetVisible(m_entries[m_selectedIndex], 0, 20);
    }
}

void Leaderboard::selectNext()
{
    if (m_isEndgameMode) {
        m_selectedIndex = qMin(m_selectedIndex + 1, 1);
    } else {
        if (m_entries.isEmpty()) return;
        m_selectedIndex = qMin(m_selectedIndex + 1, m_entries.size() - 1);
    }
    updateSelectionUI();
}

void Leaderboard::selectPrevious()
{
    if (m_isEndgameMode) {
        m_selectedIndex = qMax(0, m_selectedIndex - 1);
    } else {
        if (m_entries.isEmpty()) return;
        m_selectedIndex = qMax(0, m_selectedIndex - 1);
    }
    updateSelectionUI();
}

void Leaderboard::activateSelected()
{
    if (m_isEndgameMode) {
        if (m_selectedIndex == 0 && m_restartButton) {
            m_restartButton->click();
        } else if (m_selectedIndex == 1 && m_quitButton) {
            m_quitButton->click();
        }
    }
}

void Leaderboard::setEndgameMode(bool endgame)
{
    m_isEndgameMode = endgame;
    m_scroll->setVisible(true);
    m_buttonContainer->setVisible(endgame);
    
    if (endgame) {
        m_selectedIndex = 0;
    } else {
        m_selectedIndex = m_entries.isEmpty() ? -1 : 0;
    }
    updateSelectionUI();
}

bool Leaderboard::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::Wheel) {
        QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
        if (m_scroll && m_scroll->isVisible()) {
            int delta = wheelEvent->angleDelta().y();
            m_scroll->verticalScrollBar()->setValue(m_scroll->verticalScrollBar()->value() - delta);
        }
        event->accept();
        return true;
    }
    return QWidget::eventFilter(watched, event);
}

void Leaderboard::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateFontSizes();
}