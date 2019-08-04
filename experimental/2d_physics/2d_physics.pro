
include(../../common.pri)

QT += core gui widgets

TARGET = 2d_physics
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
    main.cpp \
    main_window.cpp \
    physics.cpp \
    sandbox_window.cpp \
    scene1.cpp \
    scene6.cpp

HEADERS += \
    main_window.h \
    physics.h \
    sandbox_factory.h \
    sandbox_window.h \
    scene1.h \
    scene6.h \
    script.h
    
FORMS += main_window.ui

addLibrary(../../registry)
addLibrary(../../core_ui)
addLibrary(../../core)
