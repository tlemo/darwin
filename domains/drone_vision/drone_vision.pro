
include(../../common.pri)

QT -= core gui

TARGET = drone_vision
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    drone.cpp \
    drone_vision.cpp \
    agent.cpp \
    scene.cpp

HEADERS += \
    drone.h \
    drone_vision.h \
    agent.h \
    scene.h

addLibrary(../../core)
