#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QList>
#include "player.h"

class JsonParser {
public:
    // Sauvegarde les données en extrayant les infos du Player
    static void savePlayerResult(Player* player, int level, int score);

    // Lit, interprète et trie la liste des joueurs par points (descendant)
    static QList<QJsonObject> readAllSorted();
};

#endif // JSONPARSER_H
