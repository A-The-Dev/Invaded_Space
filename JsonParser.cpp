#include "JsonParser.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>

void JsonParser::savePlayerResult(Player* player, int level, int score)
{
    if (!player) return;

    QFile file("leaderboard.json");
    QJsonArray rootArray;

    if (file.open(QIODevice::ReadOnly)) {
        // Utilisation de readAll() au lieu de read()
        rootArray = QJsonDocument::fromJson(file.readAll()).array();
        file.close();
    }

    QJsonObject newEntry;
    newEntry["name"] = QString::fromStdString(player->getPlayerName());
    newEntry["hp"] = player->getMaxHealth();
    newEntry["lvl"] = level;
    newEntry["atk"] = player->getAttackDamage();
    newEntry["score"] = score;
    newEntry["color"] = "#CC9900";

    rootArray.append(newEntry);

    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(rootArray).toJson());
        file.close();
    }
}

QList<QJsonObject> JsonParser::readAllSorted() {
    QList<QJsonObject> sortedList;
    QFile file("leaderboard.json");

    if (file.open(QIODevice::ReadOnly)) {
        // Utilisation de readAll() ici aussi
        QJsonArray array = QJsonDocument::fromJson(file.readAll()).array();
        for (const QJsonValue& v : array) {
            sortedList.append(v.toObject());
        }
        file.close();
    }

    std::sort(sortedList.begin(), sortedList.end(), [](const QJsonObject& a, const QJsonObject& b) {
        return a["score"].toInt() > b["score"].toInt();
        });

    return sortedList;
}