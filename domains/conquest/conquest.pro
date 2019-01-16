
include(../../common.pri)

QT -= core gui

TARGET = conquest
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    conquest.cpp \
    board.cpp \
    game.cpp \
    ann_player.cpp \
    test_players.cpp

HEADERS += \
    conquest.h \
    board.h \
    game.h \
    player.h \
    ann_player.h \
    test_players.h

addLibrary(../../core)
