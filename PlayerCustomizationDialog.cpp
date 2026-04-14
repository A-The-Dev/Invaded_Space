#include "PlayerCustomizationDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QColorDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QPainter>

PlayerCustomizationDialog::PlayerCustomizationDialog(QWidget *parent)
    : QWidget(parent),
      m_nameInput(nullptr),
      m_colorButton(nullptr),
      m_startButton(nullptr),
      m_colorPreview(nullptr),
      m_selectedColor(255, 100, 100, 180) // Default: semi-transparent red
{
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_StyledBackground);
    setFocusPolicy(Qt::StrongFocus);
    
    setupUI();
    updateColorPreview();
}

void PlayerCustomizationDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    // Title
    QLabel *titleLabel = new QLabel("Customize Your Ship", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("QLabel { color: white; }");
    mainLayout->addWidget(titleLabel);

    // Name input section
    QLabel *nameLabel = new QLabel("Enter your name:", this);
    nameLabel->setStyleSheet("QLabel { color: white; }");
    mainLayout->addWidget(nameLabel);

    m_nameInput = new QLineEdit(this);
    m_nameInput->setPlaceholderText("Player Name");
    m_nameInput->setMaxLength(20);
    m_nameInput->setStyleSheet(
        "QLineEdit {"
        "  color: white;"
        "  background: rgba(30, 30, 50, 200);"
        "  border: 2px solid rgba(100, 100, 150, 150);"
        "  border-radius: 5px;"
        "  padding: 8px;"
        "}"
        "QLineEdit:focus {"
        "  border: 2px solid rgba(150, 150, 200, 200);"
        "}"
    );
    mainLayout->addWidget(m_nameInput);

    // Color selection section
    QLabel *colorLabel = new QLabel("Choose ship color:", this);
    colorLabel->setStyleSheet("QLabel { color: white; }");
    mainLayout->addWidget(colorLabel);

    QHBoxLayout *colorLayout = new QHBoxLayout();
    colorLayout->setSpacing(15);

    m_colorButton = new QPushButton("Pick Color", this);
    m_colorButton->setStyleSheet(
        "QPushButton {"
        "  color: white;"
        "  background: rgba(30, 30, 50, 200);"
        "  border: 2px solid rgba(100, 100, 150, 150);"
        "  border-radius: 5px;"
        "  padding: 8px;"
        "}"
        "QPushButton:hover {"
        "  background: rgba(60, 60, 90, 220);"
        "}"
    );
    connect(m_colorButton, &QPushButton::clicked, this, &PlayerCustomizationDialog::onColorButtonClicked);
    colorLayout->addWidget(m_colorButton);

    m_colorPreview = new QLabel(this);
    m_colorPreview->setFixedSize(100, 40);
    m_colorPreview->setFrameStyle(QFrame::Box | QFrame::Plain);
    colorLayout->addWidget(m_colorPreview);

    mainLayout->addLayout(colorLayout);

    mainLayout->addStretch();

    // Start button
    m_startButton = new QPushButton("Start Game", this);
    m_startButton->setStyleSheet(
        "QPushButton {"
        "  color: white;"
        "  background: rgba(30, 130, 50, 220);"
        "  border-radius: 8px;"
        "  padding: 12px;"
        "  font-weight: bold;"
        "  font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "  background: rgba(40, 160, 60, 240);"
        "}"
    );
    connect(m_startButton, &QPushButton::clicked, this, &PlayerCustomizationDialog::onStartClicked);
    mainLayout->addWidget(m_startButton);

    // Set dialog background
    setStyleSheet("PlayerCustomizationDialog { background: rgba(10, 10, 30, 240); border-radius: 10px; }");
}

void PlayerCustomizationDialog::updateColorPreview()
{
    if (!m_colorPreview) return;
    
    // Create a checkerboard pattern background to show transparency
    QPixmap preview(100, 40);
    preview.fill(Qt::transparent);
    
    QPainter painter(&preview);
    
    // Draw checkerboard background
    int squareSize = 10;
    for (int y = 0; y < 40; y += squareSize)
    {
        for (int x = 0; x < 100; x += squareSize)
        {
            bool isEven = ((x / squareSize) + (y / squareSize)) % 2 == 0;
            painter.fillRect(x, y, squareSize, squareSize, 
                           isEven ? QColor(80, 80, 80) : QColor(120, 120, 120));
        }
    }
    
    // Draw the color with transparency
    painter.fillRect(0, 0, 100, 40, m_selectedColor);
    
    // Draw border
    painter.setPen(QPen(Qt::white, 2));
    painter.drawRect(0, 0, 99, 39);
    
    painter.end();
    
    m_colorPreview->setPixmap(preview);
    
    // Update tooltip with color info
    QString colorInfo = QString("RGB(%1, %2, %3) Alpha: %4\nHex: #%5")
        .arg(m_selectedColor.red())
        .arg(m_selectedColor.green())
        .arg(m_selectedColor.blue())
        .arg(m_selectedColor.alpha())
        .arg(QString("%1%2%3%4")
            .arg(m_selectedColor.red(), 2, 16, QChar('0'))
            .arg(m_selectedColor.green(), 2, 16, QChar('0'))
            .arg(m_selectedColor.blue(), 2, 16, QChar('0'))
            .arg(m_selectedColor.alpha(), 2, 16, QChar('0')).toUpper());
    
    m_colorPreview->setToolTip(colorInfo);
}

void PlayerCustomizationDialog::onColorButtonClicked()
{
    // Create color dialog without parent to avoid geometry warnings
    QColorDialog colorDialog(m_selectedColor);
    colorDialog.setOption(QColorDialog::ShowAlphaChannel, true);
    colorDialog.setWindowTitle("Choose Ship Color");
    
    // Make it modal to the application
    colorDialog.setWindowModality(Qt::ApplicationModal);
    
    if (colorDialog.exec() == QDialog::Accepted)
    {
        m_selectedColor = colorDialog.currentColor();
        
        // Ensure some transparency (alpha between 100-220)
        int alpha = m_selectedColor.alpha();
        if (alpha > 220) alpha = 220;
        if (alpha < 100) alpha = 180;
        m_selectedColor.setAlpha(alpha);
        
        updateColorPreview();
    }
}

void PlayerCustomizationDialog::onStartClicked()
{
    QString name = m_nameInput->text().trimmed();
    if (name.isEmpty())
    {
        name = "Player";
        m_nameInput->setText(name);
    }
    
    emit customizationComplete(name, m_selectedColor);
}

QString PlayerCustomizationDialog::getPlayerName() const
{
    return m_nameInput->text().trimmed().isEmpty() 
        ? "Player" 
        : m_nameInput->text().trimmed();
}

QColor PlayerCustomizationDialog::getPlayerColor() const
{
    return m_selectedColor;
}
