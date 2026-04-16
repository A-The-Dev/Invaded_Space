#include <QApplication>
#include "game.h"
#include "menu.h"
#include "PlayerCustomizationDialog.h"
#include <QTimer>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);


    Game* game = new Game(nullptr, /*startPaused=*/true);
    game->resize(1024, 768);
    game->show();

    Menu* menu = new Menu(game);
    menu->setGeometry(0, 0, game->width(), game->height());
    menu->show();
    menu->raise();

    QObject::connect(menu, &Menu::startGameRequested, game, [menu, game]() {
        if (!game) return;
        
        // Hide the menu temporarily
        menu->hide();
        
        // Create customization dialog as overlay widget
        PlayerCustomizationDialog* customDialog = new PlayerCustomizationDialog(game);
        
        // Center the customization form
        int dialogWidth = 500;
        int dialogHeight = 400;
        int x = (game->width() - dialogWidth) / 2;
        int y = (game->height() - dialogHeight) / 2;
        customDialog->setGeometry(x, y, dialogWidth, dialogHeight);
        
        customDialog->show();
        customDialog->raise();
        customDialog->setFocus();
        
        // Connect completion signal
        QObject::connect(customDialog, &PlayerCustomizationDialog::customizationComplete, 
            game, [game, customDialog](const QString &name, const QColor &color) {
            
            // Apply player customization
            if (game->getPlayer()) {
                game->getPlayer()->setPlayerName(name.toStdString());
                game->getPlayer()->setShipColor(color);
            }
            
            // Start the game
            game->startGame();
            game->setFocus();
            
            // Clean up the dialog
            customDialog->deleteLater();
        });
        
        // Handle if user somehow closes without completing
        QObject::connect(customDialog, &QObject::destroyed, game, [menu, game]() {
            // If game hasn't started yet, show menu again
            if (game && !game->property("gameStarted").toBool()) {
                if (menu) {
                    menu->show();
                    menu->raise();
                }
            }
        });
    });

    // Apply fullscreen on the Game, then reposition the menu overlay after the window has resized.
    QObject::connect(menu, &Menu::fullscreenToggled, [game, menu](bool fullscreen) {
        if (!game || !menu) return;

        if (fullscreen)
            game->showFullScreen();
        else
            game->showNormal();

        QTimer::singleShot(0, menu, [menu, game]() {
            if (game) {
                menu->setGeometry(0, 0, game->width(), game->height());
            }
        });
    });

    QObject::connect(menu, &Menu::volumeChanged, [](int value) {
        Q_UNUSED(value);
    });

    return a.exec();
}