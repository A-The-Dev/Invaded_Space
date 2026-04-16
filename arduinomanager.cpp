#include "arduinomanager.h"
#include <QDebug>

ArduinoManager::ArduinoManager(QObject *parent) : QObject(parent) {
    serial = new QSerialPort(this);
    connect(serial, &QSerialPort::readyRead, this, &ArduinoManager::readSerial);
}

bool ArduinoManager::connectToArduino(const QString &portName) {
    serial->setPortName(portName);
    serial->setBaudRate(QSerialPort::Baud115200); // Doit être le même que Serial.begin() sur Arduino
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    /*if (serial->open(QIODevice::ReadWrite)) {
        qDebug() << "SUCCÈS : Le port" << portName << "est ouvert !";
        // TRÈS IMPORTANT pour l'Arduino Mega (S2-P06)
        serial->setDataTerminalReady(true);
        return true;
    } else {
        // Si tu vois ce message, regarde l'erreur (ex: Access Denied = port utilisé par l'IDE Arduino)
        qDebug() << "ÉCHEC : Impossible d'ouvrir le port." << serial->errorString();
        return false;
    }*/
    if (serial->open(QIODevice::ReadWrite)) {
        qDebug() << "Connecté à l'Arduino sur" << portName;
        return true;
    } else {
        qDebug() << "Erreur : Impossible d'ouvrir le port" << portName;
        return false;
    }
    // Assurez-vous d'utiliser ReadWrite et non ReadOnly
    if (serial->open(QIODevice::ReadWrite)) {
        serial->setDataTerminalReady(true); // Crucial pour réveiller la Mega
        qDebug() << "Port ouvert en Lecture/Écriture";
    }
}

void ArduinoManager::readSerial() {
    // On lit toutes les données disponibles jusqu'au saut de ligne (\n)
    /*QByteArray rawData = serial->readLine();

    if (!rawData.isEmpty()) {
        //qDebug() << "BRUT REÇU :" << rawData;
    }*/
    while (serial->canReadLine()) {
        QByteArray data = serial->readLine().trimmed();
        if (data.isEmpty()) continue;

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isNull() && doc.isObject()) {
            QJsonObject obj = doc.object();
            double x = obj["direction"].toDouble();
            double y = obj["vitesse"].toDouble();
            bool ultimate = obj["accel"].toBool();
            bool tir = obj["bouton2"].toBool();
            bool grenade = obj["bouton3"].toBool();
            bool BossSpawn = obj["compteur"].toBool();

            // On envoie le signal vers player.cpp
            emit commandReceived(x, y, tir ,ultimate, grenade, BossSpawn);
        }
    }
}
void ArduinoManager::sendGameState(int level, int bossID, bool del) {
    // Envoyer des infos vers l'Arduino

    // On vérifie si le port est bien ouvert avant d'écrire
    if (serial && serial->isOpen()) {

        QJsonObject obj;
        obj["niveau"] = level;
        obj["ecran"] = bossID;
        obj["delB"] = del;

        QByteArray data = QJsonDocument(obj).toJson(QJsonDocument::Compact) + "\n";

        serial->write(data);
        serial->flush();
        //qDebug() << "Données envoyées :" << data;
    }
    else {
        //qDebug() << "Erreur : Le port série n'est pas ouvert !";
    }
}
