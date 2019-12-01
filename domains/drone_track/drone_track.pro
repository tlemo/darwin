
include(../../common.pri)

QT -= core gui

TARGET = drone_track
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    domain.cpp \
    scene.cpp \
    track.cpp

HEADERS += \
    domain.h \
    scene.h \
    track.h

addLibrary(../../core)
