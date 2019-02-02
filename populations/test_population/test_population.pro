
include(../../common.pri)

QT -= core gui

TARGET = test_population
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    test_population.cpp \
    population.cpp \
    genotype.cpp \
    brain.cpp

HEADERS += \
    test_population.h \
    population.h \
    genotype.h \
    brain.h

addLibrary(../../core)
