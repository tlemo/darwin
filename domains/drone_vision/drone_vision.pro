
include(../../common.pri)

QT -= core gui

TARGET = drone_vision
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    drone_vision.cpp \
    agent.cpp \
    scene.cpp

HEADERS += \
    drone_vision.h \
    agent.h \
    scene.h

addLibrary(../../core)
