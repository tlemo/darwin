
include(../../common.pri)

QT -= core gui

TARGET = harvester
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    robot.cpp \
    world.cpp \
    world_map.cpp \
    harvester.cpp

HEADERS += \
    robot.h \
    world.h \
    world_map.h \
    harvester.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

addLibrary(../../core)
