
include(../../common.pri)

QT += core gui charts widgets

TARGET = double_cart_pole_ui
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    double_cart_pole_ui.cpp \
    sandbox_window.cpp

HEADERS += \
    double_cart_pole_ui.h \
    sandbox_window.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

addLibrary(../double_cart_pole)
addLibrary(../../core_ui)
addLibrary(../../core)
