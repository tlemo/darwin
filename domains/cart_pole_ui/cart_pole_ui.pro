
include(../../common.pri)

QT += core gui charts widgets

TARGET = cart_pole_ui
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    cart_pole_ui.cpp \
    sandbox_window.cpp

HEADERS += \
    cart_pole_ui.h \
    sandbox_window.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

addLibrary(../cart_pole)
addLibrary(../../core_ui)
addLibrary(../../core)
