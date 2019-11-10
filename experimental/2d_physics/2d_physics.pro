
include(../../common.pri)

QT += core gui widgets

TARGET = 2d_physics
TEMPLATE = app

#CONFIG += console

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
    main.cpp \
    main_window.cpp \
    new_sandbox_dialog.cpp \
    new_scene_dialog.cpp \
    sandbox_window.cpp \
    scene_bouncing.cpp \
    scene_cart_pole.cpp \
    scene_drone.cpp \
    scene_hectic_drone.cpp \
    scene_electric_fence.cpp \
    scene_particles.cpp \
    tool_window.cpp \
    accelerometer_window.cpp \
    camera_window.cpp \
    compass_window.cpp \
    touch_window.cpp
    

HEADERS += \
    main_window.h \
    new_sandbox_dialog.h \
    new_scene_dialog.h \
    sandbox_factory.h \
    sandbox_window.h \
    scene_bouncing.h \
    scene_cart_pole.h \
    scene_drone.h \
    scene_hectic_drone.h \
    scene_electric_fence.h \
    scene_particles.h \
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
