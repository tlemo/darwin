
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

addLibrary(../domains/cart_pole_ui)
addLibrary(../domains/cart_pole)
addLibrary(../domains/conquest_ui)
addLibrary(../domains/conquest)
addLibrary(../domains/harvester_ui)
addLibrary(../domains/harvester)
addLibrary(../domains/max)
addLibrary(../domains/pong_ui)
addLibrary(../domains/pong)
addLibrary(../domains/tic_tac_toe_ui)
addLibrary(../domains/tic_tac_toe)

addLibrary(../populations/classic)
addLibrary(../populations/neat)

addLibrary(../core)
