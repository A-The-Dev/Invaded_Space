#ifndef PLAYERCUSTOMIZATIONDIALOG_H
#define PLAYERCUSTOMIZATIONDIALOG_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QColor>

class PlayerCustomizationDialog : public QWidget
{
    Q_OBJECT
public:
    explicit PlayerCustomizationDialog(QWidget *parent = nullptr);

    QString getPlayerName() const;
    QColor getPlayerColor() const;

signals:
    void customizationComplete(const QString &name, const QColor &color);
    void cancelled();

private slots:
    void onColorButtonClicked();
    void onStartClicked();

private:
    void setupUI();
    void updateColorPreview();

    QLineEdit *m_nameInput;
    QPushButton *m_colorButton;
    QPushButton *m_startButton;
    QLabel *m_colorPreview;
    QColor m_selectedColor;
};

#endif // PLAYERCUSTOMIZATIONDIALOG_H

