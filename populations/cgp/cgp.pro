
include(../../common.pri)

QT -= core gui

TARGET = cgp
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    cgp.cpp \
    population.cpp \
    genotype.cpp \
    brain.cpp \
    roulette_selection.cpp \
    cgp_islands_selection.cpp

HEADERS += \
    cgp.h \
    population.h \
    genotype.h \
    brain.h \
    roulette_selection.h \
    cgp_islands_selection.h

addLibrary(../../core)
