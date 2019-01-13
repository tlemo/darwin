
include(../../common.pri)

QT += core gui charts widgets

TARGET = conquest_ui
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    new_sandbox_dialog.cpp \
    conquest_ui.cpp \
    board_widget.cpp \
    game_stats_window.cpp \
    game_window.cpp

HEADERS += \
    new_sandbox_dialog.h \
    conquest_ui.h \
    board_widget.h \
    game_stats_window.h \
    game_window.h

FORMS += \
    new_sandbox_dialog.ui \
    game_stats_window.ui \
    game_window.ui

unix {
    target.path = /usr/lib
    INSTALLS += target
}

addLibrary(../../core)
addLibrary(../../core_ui)
addLibrary(../conquest)
