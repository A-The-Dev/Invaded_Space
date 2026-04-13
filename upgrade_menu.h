#ifndef UPGRADE_MENU_H
#define UPGRADE_MENU_H

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QResizeEvent>
#include <QEvent>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <QParallelAnimationGroup>

class UpgradeMenu : public QDialog
{
    Q_OBJECT
public:
    explicit UpgradeMenu(QWidget *parent = nullptr);
    ~UpgradeMenu();

signals:
    void upgradeSelected(int choice);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void layoutResponsive();
    void updateCenteredGeometry();
    void setupTitleAnimation();
    void setupEntranceAnimation();

    int selectedOption = 0;
    bool joystickAuNeutre = true;

    QLabel* title = nullptr;
    QPushButton* btnSpeed = nullptr;
    QPushButton* btnDamage = nullptr;
    QPushButton* btnHealth = nullptr;

    QTimer* m_animationTimer = nullptr;
    QGraphicsOpacityEffect* m_glowEffect = nullptr;
    QParallelAnimationGroup* m_entranceAnimation = nullptr;
    qreal m_glowIntensity = 0.0;

public slots:
    void navigateWithJoystick(double x, double y, bool tir, bool ulti);

private slots:
    void onAnimationTick();
};
#endif // UPGRADE_MENU_H
