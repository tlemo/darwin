
include(../../common.pri)

QT += core gui widgets

TARGET = 2d_physics
TEMPLATE = app

#CONFIG += console

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
    camera.cpp \
    touch_sensor.cpp \
    camera_window.cpp \
    touch_window.cpp \
    main.cpp \
    main_window.cpp \
    new_sandbox_dialog.cpp \
    new_scene_dialog.cpp \
    physics.cpp \
    sandbox_window.cpp \
    scene_bouncing.cpp \
    scene_cart_pole.cpp \
    scene_drone.cpp \
    scene_electric_fence.cpp \
    scene_particles.cpp \
    tool_window.cpp

HEADERS += \
    camera.h \
    touch_sensor.h \
    camera_window.h \
    touch_window.h \
    main_window.h \
    new_sandbox_dialog.h \
    new_scene_dialog.h \
    physics.h \
    sandbox_factory.h \
    sandbox_window.h \
    scene_bouncing.h \
    scene_cart_pole.h \
    scene_drone.h \
    scene_electric_fence.h \
    scene_particles.h \
    script.h \
    tool_window.h
    
FORMS += main_window.ui \
    new_sandbox_dialog.ui \
    new_scene_dialog.ui

addLibrary(../../registry)
addLibrary(../../core_ui)
addLibrary(../../core)
