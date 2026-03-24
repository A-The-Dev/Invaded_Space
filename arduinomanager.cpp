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

    if (serial->open(QIODevice::ReadWrite)) {
        qDebug() << "Connecté à l'Arduino sur" << portName;
        return true;
    } else {
        qDebug() << "Erreur : Impossible d'ouvrir le port" << portName;
        return false;
    }
}

void ArduinoManager::readSerial() {
    // On lit toutes les données disponibles jusqu'au saut de ligne (\n)
    while (serial->canReadLine()) {
        QByteArray data = serial->readLine().trimmed();

        // On essaie de parser le JSON reçu
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isNull() && doc.isObject()) {
            QJsonObject obj = doc.object();

            // Extraction des valeurs
            double angle = obj["angle"].toDouble();
            double vitesse = obj["vitesse"].toDouble();
            bool tir = obj["tir"].toBool();

            // On envoie le signal vers player.cpp
            emit commandReceived(angle, vitesse, tir);
        }
    }
}

void ArduinoManager::sendGameState(int level, int bossID) {
    // Envoyer des infos vers l'Arduino
    QJsonObject obj;
    obj["level"] = level;
    obj["boss"] = bossID;
    serial->write(QJsonDocument(obj).toJson(QJsonDocument::Compact) + "\n");
}
