
include(../common.pri)

QT += core gui charts widgets

TARGET = darwin_studio
TEMPLATE = app
CONFIG += link_prl
#CONFIG += console

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    main_window.cpp \
    evolution_window.cpp \
    output_window.cpp \
    fitness_window.cpp \
    perf_window.cpp \
    experiment_window.cpp \
    settings.cpp \
    new_experiment_dialog.cpp \
    open_experiment_dialog.cpp \
    start_evolution_dialog.cpp

HEADERS += \
    main_window.h \
    evolution_window.h \
    output_window.h \
    fitness_window.h \
    pubsub_relay.h \
    perf_window.h \
    experiment_window.h \
    settings.h \
    new_experiment_dialog.h \
    open_experiment_dialog.h \
    console_buffer.h \
    start_evolution_dialog.h

FORMS += \
    main_window.ui \
    evolution_window.ui \
    output_window.ui \
    fitness_window.ui \
    perf_window.ui \
    experiment_window.ui \
    new_experiment_dialog.ui \
    open_experiment_dialog.ui \
    start_evolution_dialog.ui

addLibrary(../registry)
addLibrary(../core_ui)
addLibrary(../core)
