
include(../../common.pri)

QT -= core gui

TARGET = max
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    robot.cpp \
    world.cpp

HEADERS += \
    robot.h \
    world.h \
    max.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

addLibrary(../../core)
