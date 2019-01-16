
include(../../common.pri)

QT -= core gui

TARGET = unicycle
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    unicycle.cpp \
    agent.cpp \
    world.cpp

HEADERS += \
    unicycle.h \
    agent.h \
    world.h

addLibrary(../../core)
