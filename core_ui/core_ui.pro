
include(../common.pri)

QT += core gui charts widgets

TARGET = core_ui
TEMPLATE = lib
CONFIG += staticlib

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    canvas.cpp \
    properties_widget.cpp \
    physics/box2d_renderer.cpp \
    physics/box2d_sandbox_dialog.cpp \
    physics/box2d_sandbox_window.cpp \
    physics/box2d_widget.cpp \
    physics/accelerometer_widget.cpp \
    physics/camera_widget.cpp \
    physics/compass_widget.cpp \
    physics/touch_widget.cpp

HEADERS += \
    canvas.h \
    properties_widget.h \
    physics/box2d_renderer.h \
    physics/box2d_sandbox_dialog.h \
    physics/box2d_sandbox_window.h \
    physics/box2d_widget.h \
    physics/accelerometer_widget.h \
    physics/camera_widget.h \
    physics/compass_widget.h \
    physics/touch_widget.h

FORMS += \
    physics/box2d_sandbox_dialog.ui \
    physics/box2d_sandbox_window.ui

RESOURCES += \
    resources.qrc

addLibrary(../core)
