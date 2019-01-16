
include(../../common.pri)

QT += core gui charts widgets

TARGET = unicycle_ui
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    unicycle_ui.cpp \
    sandbox_window.cpp

HEADERS += \
    unicycle_ui.h \
    sandbox_window.h

addLibrary(../unicycle)
addLibrary(../../core_ui)
addLibrary(../../core)
