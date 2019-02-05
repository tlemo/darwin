
include(../../common.pri)

QT -= core gui

TARGET = test_domain
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    test_domain.cpp \
    agent.cpp

HEADERS += \
    test_domain.h \
    agent.h

addLibrary(../../core)
