#include <QApplication>
#include "game.h"
#include "menu.h"
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
        game->startGame();
        menu->hide();
        game->setFocus();
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