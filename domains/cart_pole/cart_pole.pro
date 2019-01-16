
include(../../common.pri)

QT -= core gui

TARGET = cart_pole
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    cart_pole.cpp \
    agent.cpp \
    world.cpp

HEADERS += \
    cart_pole.h \
    agent.h \
    world.h

addLibrary(../../core)
