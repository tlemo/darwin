
include(../../common.pri)

QT -= core gui

TARGET = car_track
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    domain.cpp \
    scene.cpp

HEADERS += \
    domain.h \
    scene.h

addLibrary(../../core)
