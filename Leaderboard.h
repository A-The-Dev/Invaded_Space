#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <QWidget>
#include <QFrame>
#include <QJsonObject>
#include <QEvent>
#include <QVector>

class QScrollArea;
class QVBoxLayout;
class QPushButton;

class Leaderboard : public QWidget {
    Q_OBJECT
public:
    explicit Leaderboard(QWidget* parent = nullptr);

    // Navigation API used by Menu keyboard handling
    void selectNext();
    void selectPrevious();
    void activateSelected();
    void refresh(); // reload entries

    // Endgame mode: show restart/quit buttons below leaderboard
    void setEndgameMode(bool endgame);
    bool isEndgameMode() const { return m_isEndgameMode; }

signals:
    void restartRequested();
    void quitRequested();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    QFrame* createEntryWidget(const QJsonObject& obj);
    void updateSelectionUI();
    void updateFontSizes();
    
    QVector<QFrame*> m_entries;
    QScrollArea* m_scroll;
    QWidget* m_container;
    QVBoxLayout* m_listLayout;
    int m_selectedIndex;
    
    // Endgame mode widgets
    QWidget* m_buttonContainer;
    QVBoxLayout* m_buttonLayout;
    QPushButton* m_restartButton;
    QPushButton* m_quitButton;
    bool m_isEndgameMode;
};

#endif