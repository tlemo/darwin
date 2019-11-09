
include(../../common.pri)

QT -= core gui

TARGET = drone_vision
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    drone.cpp \
    agent.cpp \
    vision\domain.cpp \
    vision\scene.cpp

HEADERS += \
    drone.h \
    agent.h \
    vision\domain.h \
    vision\scene.h

addLibrary(../../core)
