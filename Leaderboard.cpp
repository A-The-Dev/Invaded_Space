#include "leaderboard.h"
#include "JsonParser.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QWheelEvent>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QFile> // added

Leaderboard::Leaderboard(QWidget* parent) : QWidget(parent), m_scroll(nullptr),
    m_container(nullptr), m_listLayout(nullptr), m_selectedIndex(-1)
{
    setStyleSheet("background-color: rgba(15, 15, 15, 240); border: 2px solid white; border-radius: 10px;");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12,12,12,12);
    mainLayout->setSpacing(8);

    QLabel* title = new QLabel("LEADERBOARD", this);
    title->setStyleSheet("color: white; font-size: 22pt; font-weight: bold;");
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
    mainLayout->addWidget(m_scroll);

    m_scroll->viewport()->installEventFilter(this);

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
    iconLabel->setFixedSize(70, 70);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("border: 1px solid #444; background: #050505;");

    QPixmap playerImg;
    playerImg.load("./resources/spaceship.png");

    if (!playerImg.isNull()) {
        iconLabel->setPixmap(playerImg.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        iconLabel->setText("?");
        iconLabel->setStyleSheet("color: white; font-weight: bold; border: 1px solid #444;");
    }

    QLabel* stats = new QLabel(QString("<b style='font-size:13pt;'>%1</b><br/>HP:%2 | LVL:%3 | ATK:%4")
        .arg(obj["name"].toString()).arg(obj["hp"].toInt()).arg(obj["lvl"].toInt()).arg(obj["atk"].toInt()), frame);
    stats->setTextFormat(Qt::RichText);

    QLabel* score = new QLabel(QString("<b style='font-size:11pt; color:#FF5252;'>%1</b>").arg(obj["score"].toInt()), frame);
    score->setAlignment(Qt::AlignCenter);

    layout->addWidget(iconLabel);
    layout->addWidget(stats, 1);
    layout->addStretch();
    layout->addWidget(score);
    return frame;
}

void Leaderboard::updateSelectionUI()
{
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
    if (m_entries.isEmpty()) return;
    m_selectedIndex = qMin(m_selectedIndex + 1, m_entries.size() - 1);
    updateSelectionUI();
}

void Leaderboard::selectPrevious()
{
    if (m_entries.isEmpty()) return;
    m_selectedIndex = qMax(0, m_selectedIndex - 1);
    updateSelectionUI();
}

void Leaderboard::activateSelected()
{
    // No specific activation defined. Could display details in future.
}

bool Leaderboard::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::Wheel) {
        QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
        if (m_scroll) {
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
    // Let layouts control internal sizes; no fixed widths here.
    // Optionally adjust font sizes or entry paddings based on event->size()
}