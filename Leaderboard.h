#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <QWidget>
#include <QFrame>
#include <QJsonObject>
#include <QEvent>
#include <QVector>

class QScrollArea;
class QVBoxLayout;

class Leaderboard : public QWidget {
    Q_OBJECT
public:
    explicit Leaderboard(QWidget* parent = nullptr);

    // Navigation API used by Menu keyboard handling
    void selectNext();
    void selectPrevious();
    void activateSelected();
    void refresh(); // reload entries

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    QFrame* createEntryWidget(const QJsonObject& obj);
    void updateSelectionUI();
    QVector<QFrame*> m_entries;
    QScrollArea* m_scroll;
    QWidget* m_container;
    QVBoxLayout* m_listLayout;
    int m_selectedIndex;
};

#endif