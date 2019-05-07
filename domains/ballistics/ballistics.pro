
include(../../common.pri)

QT -= core gui

TARGET = ballistics
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    ballistics.cpp \
    agent.cpp \
    world.cpp

HEADERS += \
    ballistics.h \
    agent.h \
    world.h

addLibrary(../../core)
