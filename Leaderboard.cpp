#include "leaderboard.h"
#include "JsonParser.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QWheelEvent>
#include <QScrollBar>

Leaderboard::Leaderboard(QWidget* parent) : QWidget(parent)
{
    setFixedSize(620, 500);
    setStyleSheet("background-color: rgba(15, 15, 15, 240); border: 2px solid white; border-radius: 10px;");

    auto* mainLayout = new QVBoxLayout(this);
    QLabel* title = new QLabel("LEADERBOARD");
    title->setStyleSheet("color: white; font-size: 22pt; font-weight: bold; border: none; background: none;");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    QScrollArea* scroll = new QScrollArea(this);
    QWidget* container = new QWidget();
    QVBoxLayout* listLayout = new QVBoxLayout(container);

    QList<QJsonObject> players = JsonParser::readAllSorted();
    for (const QJsonObject& obj : players) {
        listLayout->addWidget(createEntryWidget(obj));
    }

    listLayout->addStretch();
    scroll->setWidget(container);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("background: transparent; border: none;");

    // --- FORCE LE SCROLL ICI ---
    scroll->viewport()->installEventFilter(this);
    mainLayout->addWidget(scroll);
}

bool Leaderboard::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::Wheel) {
        QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
        QScrollArea* scroll = this->findChild<QScrollArea*>();

        if (scroll) {
            // On fait défiler manuellement la barre du leaderboard
            int delta = wheelEvent->angleDelta().y();
            scroll->verticalScrollBar()->setValue(scroll->verticalScrollBar()->value() - delta);
        }

        event->accept(); // Dit au système : "J'ai géré l'événement, ne le donne pas au jeu"
        return true;
    }
    return QWidget::eventFilter(watched, event);
}

QFrame* Leaderboard::createEntryWidget(const QJsonObject& obj) 
{
    QFrame* frame = new QFrame();
    frame->setFixedSize(550, 110);
    frame->setStyleSheet("QFrame { border: 2px solid white; background-color: #121212; margin-bottom: 10px; border-radius: 5px; }"
        "QLabel { border: none; color: white; background: none; }");

    auto* layout = new QHBoxLayout(frame);

    QLabel* iconLabel = new QLabel();
    iconLabel->setFixedSize(70, 70);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("border: 1px solid #444; background: #050505;");

    // --- TEST DES CHEMINS D'IMAGE ---
    QPixmap playerImg;
    // Test 1 : Ressource Qt (si tu as un fichier .qrc)
    playerImg.load("./resources/spaceship.png");

    // Test 2 : Dossier local (si le dossier resources est à côté du .exe)
    if (playerImg.isNull()) playerImg.load("./resources/spaceship.png");

    // Test 3 : Dossier parent (souvent nécessaire en mode Debug avec Visual Studio)
    if (playerImg.isNull()) playerImg.load("./resources/spaceship.png");

    if (!playerImg.isNull()) 
    {
        iconLabel->setPixmap(playerImg.scaled(60, 60, Qt::KeepAspectRatio, Qt::FastTransformation));
    }
    else 
    {
        iconLabel->setText("ERR"); // Affiche ERR si l'image est introuvable
        iconLabel->setStyleSheet("color: red; font-weight: bold; border: 1px solid red;");
    }

    QLabel* stats = new QLabel(QString("<b style='font-size:13pt;'>%1</b><br/>HP:%2 | LVL:%3 | ATK:%4")
        .arg(obj["name"].toString()).arg(obj["hp"].toInt()).arg(obj["lvl"].toInt()).arg(obj["atk"].toInt()));

    QLabel* score = new QLabel(QString("SCORE:<br/><span style='color:#FF5252;'>%1</span>").arg(obj["score"].toInt()));
    score->setAlignment(Qt::AlignCenter);

    layout->addWidget(iconLabel);
    layout->addSpacing(15);
    layout->addWidget(stats);
    layout->addStretch();
    layout->addWidget(score);
    layout->addSpacing(10);

    return frame;
}