
include(../../common.pri)

QT += core gui charts widgets

TARGET = ballistics_ui
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    ballistics_ui.cpp \
    sandbox_window.cpp \
    scene_ui.cpp

HEADERS += \
    ballistics_ui.h \
    sandbox_window.h \
    scene_ui.h

addLibrary(../ballistics)
addLibrary(../../core_ui)
addLibrary(../../core)
