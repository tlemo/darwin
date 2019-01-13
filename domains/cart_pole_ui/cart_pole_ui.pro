
include(../../common.pri)

QT += core gui charts widgets

TARGET = cart_pole_ui
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    cart_pole_ui.cpp \
    new_sandbox_dialog.cpp \
    world_widget.cpp \
    sandbox_window.cpp

HEADERS += \
    cart_pole_ui.h \
    new_sandbox_dialog.h \
    world_widget.h \
    sandbox_window.h

FORMS += \
    new_sandbox_dialog.ui \
    sandbox_window.ui

unix {
    target.path = /usr/lib
    INSTALLS += target
}

addLibrary(../cart_pole)
addLibrary(../../core_ui)
addLibrary(../../core)
