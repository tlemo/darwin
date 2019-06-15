
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
addLibrary(../domains/double_cart_pole_ui)
addLibrary(../domains/double_cart_pole)
addLibrary(../domains/harvester_ui)
addLibrary(../domains/harvester)
addLibrary(../domains/max)
addLibrary(../domains/pong_ui)
addLibrary(../domains/pong)
addLibrary(../domains/tic_tac_toe_ui)
addLibrary(../domains/tic_tac_toe)
addLibrary(../domains/unicycle_ui)
addLibrary(../domains/unicycle)
addLibrary(../domains/ballistics_ui)
addLibrary(../domains/ballistics)
addLibrary(../domains/test_domain)

addLibrary(../populations/cgp)
addLibrary(../populations/cne)
addLibrary(../populations/test_population)
addLibrary(../populations/neat)

addLibrary(../core)
