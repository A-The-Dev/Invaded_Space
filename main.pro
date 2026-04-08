TEMPLATE     = vcapp
TARGET       = Invaded_Space
CONFIG      += warn_on qt debug windows console

QT          += widgets serialport

# Header files
HEADERS     += arduinomanager.h \
               Boss.h \
               BossType.h \
               bullet.h \
               BulletType.h \
               collisionmanager.h \
               enemy.h \
               game.h \
               hud.h \
               levelsystem.h \
               mainwindow.h \
               player.h \
               spaceobject.h \
               ultimate.h \
               upgrade_menu.h \
               weaponpowerup.h \
               xporb.h \
               Leaderboard.h \
               JsonParser.h \
               menu.h \
	           Grenades.h

# Source files
SOURCES     += arduinomanager.cpp \
               Boss.cpp \
               bullet.cpp \
               BulletType.cpp \
               collisionmanager.cpp \
               enemy.cpp \
               game.cpp \
               hud.cpp \
               levelsystem.cpp \
               main.cpp \
               mainwindow.cpp \
               player.cpp \
               spaceobject.cpp \
               ultimate.cpp \
               upgrade_menu.cpp \
               weaponpowerup.cpp \
               xporb.cpp \
               Leaderboard.cpp \
               JsonParser.cpp \
               menu.cpp \
	           Grenades.cpp

# UI files
FORMS       += mainwindow.ui

INCLUDEPATH += .
