
include(../../common.pri)

QT += core gui widgets

TARGET = replicators
TEMPLATE = app

#CONFIG += console

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
    main.cpp \
    main_window.cpp \
    new_experiment_dialog.cpp \
    phenotype_widget.cpp \
    replicators.cpp \
    species/segment_tree.cpp

HEADERS += \
    main_window.h \
    new_experiment_dialog.h \
    phenotype_widget.h \
    replicators.h \
    species/segment_tree.h
    
FORMS += \
    main_window.ui \
    new_experiment_dialog.ui

addLibrary(../../registry)
addLibrary(../../core_ui)
addLibrary(../../core)
