
include(../../common.pri)

QT -= core gui

TARGET = cgp
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    cgp.cpp \
    population.cpp \
    genotype.cpp \
    brain.cpp

HEADERS += \
    cgp.h \
    population.h \
    genotype.h \
    brain.h

addLibrary(../../core)
