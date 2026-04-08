#include <QApplication>
#include "game.h"
#include "menu.h"
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Game *game = new Game(nullptr, /*startPaused=*/true);
    game->resize(1024, 768);
    game->show();

    Menu *menu = new Menu(game->viewport());
    menu->setGeometry(0, 0, game->viewport()->width(), game->viewport()->height());
    menu->show();

    QObject::connect(menu, &Menu::startGameRequested, [menu, game](){
        if (!game) return;
        game->startGame();
        menu->hide();
        game->setFocus();
    });

    // Apply fullscreen on the Game, then reposition the menu overlay after the window has resized.
    QObject::connect(menu, &Menu::fullscreenToggled, [game, menu](bool fullscreen){
        if (!game || !menu) return;

        if (fullscreen)
            game->showFullScreen();
        else
            game->showNormal();

        // Defer overlay resize until after Qt's layout/resize completes.
        QTimer::singleShot(0, menu, [menu]() {
            if (QWidget *parent = menu->parentWidget()) {
                menu->setGeometry(0, 0, parent->width(), parent->height());
                // Calling setGeometry triggers Menu::resizeEvent which will run layoutResponsive()
                // and deferred selector positioning.
            }
        });
    });

    QObject::connect(menu, &Menu::volumeChanged, [](int value){
        Q_UNUSED(value);
    });

    return a.exec();
}
