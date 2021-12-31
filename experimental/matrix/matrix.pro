
include(../../common.pri)

QT += core gui widgets

TARGET = matrix
TEMPLATE = app

CONFIG += console

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
    main.cpp \
    main_window.cpp \
    map_layers.cpp \
    map_scene.cpp \
    map_view.cpp \
    tool_window.cpp \
    accelerometer_window.cpp \
    camera_window.cpp \
    compass_window.cpp \
    touch_window.cpp \
    visible_world_state.cpp \
    world.cpp
    

HEADERS += \
    main_window.h \
    map_layers.h \
    map_scene.h \
    map_view.h \
    tool_window.h \
    accelerometer_window.h \
    camera_window.h \
    compass_window.h \
    touch_window.h \
    visible_world_state.h \
    world.h
    
FORMS += main_window.ui

addLibrary(../../registry)
addLibrary(../../core_ui)
addLibrary(../../core)
