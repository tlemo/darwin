
include(../../common.pri)

QT += core gui charts widgets

TARGET = cart_pole_ui
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    cart_pole_ui.cpp \
    new_sandbox_dialog.cpp \
    world_widget.cpp \
    sandbox_window.cpp \
    box2d_renderer.cpp

HEADERS += \
    cart_pole_ui.h \
    new_sandbox_dialog.h \
    world_widget.h \
    sandbox_window.h \
    box2d_renderer.h

FORMS += \
    new_sandbox_dialog.ui \
    sandbox_window.ui

unix {
    target.path = /usr/lib
    INSTALLS += target
}

addLibrary(../../core)
addLibrary(../../core_ui)
addLibrary(../cart_pole)
