
include(../../common.pri)

QT -= core gui

TARGET = drone_track
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    domain.cpp \
    scene.cpp

HEADERS += \
    domain.h \
    scene.h

addLibrary(../../core)
