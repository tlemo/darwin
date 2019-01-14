
include(../common.pri)

QT += core gui charts widgets

TARGET = core_ui
TEMPLATE = lib
CONFIG += staticlib

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    properties_widget.cpp \
    box2d_renderer.cpp \
    box2d_sandbox_dialog.cpp \
    box2d_sandbox_window.cpp \
    box2d_widget.cpp \
    canvas.cpp

HEADERS += \
    properties_widget.h \
    box2d_renderer.h \
    box2d_sandbox_dialog.h \
    box2d_sandbox_window.h \
    box2d_widget.h \
    canvas.h

FORMS += \
    box2d_sandbox_dialog.ui \
    box2d_sandbox_window.ui

addLibrary(../core)
