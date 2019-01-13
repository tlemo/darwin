
include(../common.pri)

QT -= core gui

TARGET = registry
TEMPLATE = lib
CONFIG += staticlib
CONFIG += create_prl

SOURCES += \
    registry.cpp \
    registry_ui.cpp

HEADERS += \
    registry.h \
    registry_ui.h

addLibrary(../core)
addLibrary(../populations/classic)
addLibrary(../populations/neat)
addLibrary(../domains/pong)
addLibrary(../domains/pong_ui)
addLibrary(../domains/cart_pole)
addLibrary(../domains/cart_pole_ui)
addLibrary(../domains/max)
addLibrary(../domains/tic_tac_toe)
addLibrary(../domains/tic_tac_toe_ui)
addLibrary(../domains/conquest)
addLibrary(../domains/conquest_ui)
addLibrary(../domains/harvester)
addLibrary(../domains/harvester_ui)
