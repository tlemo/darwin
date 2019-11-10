
include(../common.pri)

QT += core gui charts widgets

TARGET = core_ui
TEMPLATE = lib
CONFIG += staticlib

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    canvas.cpp \
    properties_widget.cpp \
    sim/box2d_renderer.cpp \
    sim/box2d_sandbox_dialog.cpp \
    sim/box2d_sandbox_window.cpp \
    sim/box2d_widget.cpp \
    sim/accelerometer_widget.cpp \
    sim/camera_widget.cpp \
    sim/compass_widget.cpp \
    sim/touch_widget.cpp

HEADERS += \
    canvas.h \
    properties_widget.h \
    sim/box2d_renderer.h \
    sim/box2d_sandbox_dialog.h \
    sim/box2d_sandbox_window.h \
    sim/box2d_widget.h \
    sim/accelerometer_widget.h \
    sim/camera_widget.h \
    sim/compass_widget.h \
    sim/touch_widget.h

FORMS += \
    sim/box2d_sandbox_dialog.ui \
    sim/box2d_sandbox_window.ui

RESOURCES += \
    resources.qrc

addLibrary(../core)
