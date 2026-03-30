#ifndef ARDUINOMANAGER_H
#define ARDUINOMANAGER_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QJsonDocument>
#include <QJsonObject>

class ArduinoManager : public QObject {
    Q_OBJECT
public:
    explicit ArduinoManager(QObject *parent = nullptr);
    bool connectToArduino(const QString &portName);
    void sendGameState(int level, int bossID, bool del);

signals:
    void commandReceived(double angle, double vitesse, bool tir, bool ultimate);

private slots:
    void readSerial();

private:
    QSerialPort *serial;
};
#endif // ARDUINOMANAGER_H
