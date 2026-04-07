#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <QWidget>
#include <QFrame>
#include <QJsonObject>
#include <QEvent>

class Leaderboard : public QWidget {
    Q_OBJECT
public:
    explicit Leaderboard(QWidget* parent = nullptr);

protected:
    // Cette fonction intercepte la molette avant qu'elle n'atteigne le jeu
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QFrame* createEntryWidget(const QJsonObject& obj);
};

#endif