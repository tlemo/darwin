
include(../../common.pri)

QT += core gui charts widgets

TARGET = drone_follow_ui
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    domain_ui.cpp \
    sandbox_window.cpp \
    scene_ui.cpp

HEADERS += \
    domain_ui.h \
    sandbox_window.h \
    scene_ui.h

addLibrary(../drone_follow)
addLibrary(../../core_ui)
addLibrary(../../core)
