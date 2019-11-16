
include(../../common.pri)

QT += core gui widgets

TARGET = 2d_physics
TEMPLATE = app

#CONFIG += console

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
    scenes/bouncing.cpp \
    scenes/cart_pole.cpp \
    scenes/drone.cpp \
    scenes/hectic_drone.cpp \
    scenes/electric_fence.cpp \
    scenes/particles.cpp \
    main.cpp \
    main_window.cpp \
    new_sandbox_dialog.cpp \
    new_scene_dialog.cpp \
    sandbox_window.cpp \
    tool_window.cpp \
    accelerometer_window.cpp \
    camera_window.cpp \
    compass_window.cpp \
    touch_window.cpp
    

HEADERS += \
    scenes/bouncing.h \
    scenes/cart_pole.h \
    scenes/drone.h \
    scenes/hectic_drone.h \
    scenes/electric_fence.h \
    scenes/particles.h \
    main_window.h \
    new_sandbox_dialog.h \
    new_scene_dialog.h \
    sandbox_factory.h \
    sandbox_window.h \
    script.h \
    tool_window.h \
    accelerometer_window.h \
    camera_window.h \
    compass_window.h \
    touch_window.h
    
FORMS += main_window.ui \
    new_sandbox_dialog.ui \
    new_scene_dialog.ui

addLibrary(../../registry)
addLibrary(../../core_ui)
addLibrary(../../core)
