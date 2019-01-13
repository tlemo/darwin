
include(../../common.pri)

QT -= core gui

TARGET = classic
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    classic.cpp \
    genetic_operators.cpp \
    feedforward.cpp \
    lstm.cpp \
    rnn.cpp \
    full_rnn.cpp \
    lstm_lite.cpp

HEADERS += \
    classic.h \
    population.h \
    feedforward.h \
    lstm.h \
    rnn.h \
    full_rnn.h \
    brain.h \
    lstm_lite.h \
    genotype.h

addLibrary(../../core)    
