
include(../../common.pri)

QT -= core gui

TARGET = double_cart_pole
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    double_cart_pole.cpp \
    agent.cpp \
    world.cpp

HEADERS += \
    double_cart_pole.h \
    agent.h \
    world.h

addLibrary(../../core)
