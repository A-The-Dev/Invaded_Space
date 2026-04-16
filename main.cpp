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
        
        menu->hide();
        
        // Create customization dialog
        PlayerCustomizationDialog* customDialog = new PlayerCustomizationDialog(game);

        int dialogWidth = 500;
        int dialogHeight = 400;
        int x = (game->width() - dialogWidth) / 2;
        int y = (game->height() - dialogHeight) / 2;
        customDialog->setGeometry(x, y, dialogWidth, dialogHeight);
        
        customDialog->show();
        customDialog->raise();
        customDialog->setFocus();
        
        QObject::connect(customDialog, &PlayerCustomizationDialog::customizationComplete, 
            game, [game, customDialog](const QString &name, const QColor &color) {
            
            if (game->getPlayer()) {
                game->getPlayer()->setPlayerName(name.toStdString());
                game->getPlayer()->setShipColor(color);
            }
            
            // Start the game
            game->startGame();
            game->setFocus();

            customDialog->deleteLater();
        });
        
        QObject::connect(customDialog, &QObject::destroyed, game, [menu, game]() {
            if (game && !game->property("gameStarted").toBool()) {
                if (menu) {
                    menu->show();
                    menu->raise();
                }
            }
        });
    });

    // Apply fullscreen on the Game
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