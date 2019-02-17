
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
    functions.cpp

HEADERS += \
    cgp.h \
    population.h \
    genotype.h \
    brain.h \
    functions.h

DISTFILES += \
    functions_table.def

addLibrary(../../core)
