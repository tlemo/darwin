
include(../../common.pri)

QT -= core gui

TARGET = tic_tac_toe
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    board.cpp \
    tic_tac_toe.cpp \
    ann_player.cpp \
    test_players.cpp \
    game_rules.cpp

HEADERS += \
    player.h \
    board.h \
    tic_tac_toe.h \
    ann_player.h \
    test_players.h \
    game_rules.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

addLibrary(../../core)
