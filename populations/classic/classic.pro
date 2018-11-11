
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
    
#------------------------------------------------------------------------------
# core
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../core/release/ -lcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../core/debug/ -lcore
else:unix: LIBS += -L$$OUT_PWD/../../core/ -lcore

DEPENDPATH += $$PWD/../../core

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core/release/libcore.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core/debug/libcore.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core/release/core.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core/debug/core.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../core/libcore.a

#------------------------------------------------------------------------------
# common definitions
#
include(../../common.pri)
