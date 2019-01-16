
include(../../common.pri)

QT -= core gui

TARGET = pong
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    game.cpp \
    pong.cpp \
    ann_player.cpp \
    test_players.cpp

HEADERS += \
    player.h \
    game.h \
    pong.h \
    ann_player.h \
    test_players.h

addLibrary(../../core)
