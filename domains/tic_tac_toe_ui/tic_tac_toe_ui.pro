
include(../../common.pri)

QT += core gui widgets

TARGET = tic_tac_toe_ui
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    tic_tac_toe_ui.cpp \
    new_sandbox_dialog.cpp \
    sandbox_window.cpp \
    board_widget.cpp \
    game.cpp

HEADERS += \
    tic_tac_toe_ui.h \
    new_sandbox_dialog.h \
    sandbox_window.h \
    board_widget.h \
    human_player.h \
    game.h

FORMS += \
    new_sandbox_dialog.ui \
    sandbox_window.ui

addLibrary(../../core)
addLibrary(../../core_ui)
addLibrary(../tic_tac_toe)
