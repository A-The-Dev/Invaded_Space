TEMPLATE     = vcapp
TARGET       = Invaded_Space
CONFIG      += warn_on qt debug windows console

QT          += widgets serialport multimedia

# Header files
HEADERS     += arduinomanager.h \
               Boss.h \
               bullet.h \
               collisionmanager.h \
               enemy.h \
               game.h \
               hud.h \
               levelsystem.h \
               player.h \
               spaceobject.h \
               ultimate.h \
               upgrade_menu.h \
               xporb.h \
               Leaderboard.h \
               JsonParser.h \
               menu.h \
	           Grenades.h \
               PlayerCustomizationDialog.h \
               SoundManager.h

# Source files
SOURCES     += arduinomanager.cpp \
               Boss.cpp \
               bullet.cpp \
               collisionmanager.cpp \
               enemy.cpp \
               game.cpp \
               hud.cpp \
               levelsystem.cpp \
               main.cpp \
               player.cpp \
               spaceobject.cpp \
               ultimate.cpp \
               upgrade_menu.cpp \
               xporb.cpp \
               Leaderboard.cpp \
               JsonParser.cpp \
               menu.cpp \
	           Grenades.cpp \
               PlayerCustomizationDialog.cpp \
               SoundManager.cpp

INCLUDEPATH += .
