
include(../../common.pri)

QT -= core gui

TARGET = dummy
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    dummy.cpp \
    population.cpp \
    genotype.cpp \
    brain.cpp

HEADERS += \
    dummy.h \
    population.h \
    genotype.h \
    brain.h

addLibrary(../../core)
