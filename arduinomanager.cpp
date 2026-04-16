#include "arduinomanager.h"
#include <QDebug>

ArduinoManager::ArduinoManager(QObject *parent) : QObject(parent) {
    serial = new QSerialPort(this);
    connect(serial, &QSerialPort::readyRead, this, &ArduinoManager::readSerial);
}

bool ArduinoManager::connectToArduino(const QString &portName) {
    serial->setPortName(portName);
    serial->setBaudRate(QSerialPort::Baud115200);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    if (serial->open(QIODevice::ReadWrite)) {
        qDebug() << "Connecté à l'Arduino sur" << portName;
        return true;
    } else {
        qDebug() << "Erreur : Impossible d'ouvrir le port" << portName;
        return false;
    }
}

void ArduinoManager::readSerial() {
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
			bool volume    = obj["son"].toDouble();
            bool pause = obj["bouton4"].toBool();

            // On envoie le signal vers player.cpp
            emit commandReceived(x, y, tir ,ultimate, grenade, BossSpawn, pause, volume);
        }
    }
}
void ArduinoManager::sendGameState(int level, int bossID, bool del) {
    if (serial && serial->isOpen()) {

        QJsonObject obj;
        obj["niveau"] = level;
        obj["ecran"] = bossID;
        obj["delB"] = del;

        QByteArray data = QJsonDocument(obj).toJson(QJsonDocument::Compact) + "\n";

        serial->write(data);
        serial->flush();
    }
}
