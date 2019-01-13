
include(../../common.pri)

QT -= core gui

TARGET = neat
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    neat.cpp \
    population.cpp \
    genotype.cpp \
    brain.cpp

HEADERS += \
    neat.h \
    population.h \
    genotype.h \
    brain.h

addLibrary(../../core)    
