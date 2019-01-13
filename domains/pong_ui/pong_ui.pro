
include(../../common.pri)

QT += core gui widgets

TARGET = pong_ui
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    pong_widget.cpp \
    game_widget.cpp \
    new_sandbox_dialog.cpp \
    pong_ui.cpp

HEADERS += \
    pong_widget.h \
    game_widget.h \
    human_player.h \
    pong_ui.h \
    new_sandbox_dialog.h

FORMS += \
    game_widget.ui \
    new_sandbox_dialog.ui

unix {
    target.path = /usr/lib
    INSTALLS += target
}

addLibrary(../pong)
addLibrary(../../core_ui)
addLibrary(../../core)
