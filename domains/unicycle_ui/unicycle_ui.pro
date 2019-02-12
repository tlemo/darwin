
include(../../common.pri)

QT += core gui charts widgets

TARGET = unicycle_ui
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    unicycle_ui.cpp \
    sandbox_window.cpp \
    scene_ui.cpp

HEADERS += \
    unicycle_ui.h \
    sandbox_window.h \
    scene_ui.h

addLibrary(../unicycle)
addLibrary(../../core_ui)
addLibrary(../../core)
