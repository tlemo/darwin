
include(../../common.pri)

QT -= core gui

TARGET = cne
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    cne.cpp \
    genetic_operators.cpp \
    feedforward.cpp \
    lstm.cpp \
    rnn.cpp \
    full_rnn.cpp \
    lstm_lite.cpp

HEADERS += \
    cne.h \
    population.h \
    feedforward.h \
    lstm.h \
    rnn.h \
    full_rnn.h \
    brain.h \
    lstm_lite.h \
    genotype.h

addLibrary(../../core)    
