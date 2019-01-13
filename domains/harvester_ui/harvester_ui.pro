
include(../../common.pri)

QT += core gui charts widgets

TARGET = harvester_ui
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    new_sandbox_dialog.cpp \
    harvester_ui.cpp \
    world_widget.cpp \
    sandbox_window.cpp

HEADERS += \
    new_sandbox_dialog.h \
    harvester_ui.h \
    world_widget.h \
    sandbox_window.h

FORMS += \
    new_sandbox_dialog.ui \
    sandbox_window.ui

unix {
    target.path = /usr/lib
    INSTALLS += target
}

addLibrary(../../core)
addLibrary(../../core_ui)
addLibrary(../harvester)
