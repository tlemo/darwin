
include(../common.pri)

QT -= core gui

TARGET = core
TEMPLATE = lib
CONFIG += staticlib
CONFIG += create_prl

SOURCES += \
    ann_utils.cpp \
    darwin.cpp \
    logging.cpp \
    platform_abstraction_layer.cpp \
    database.cpp \
    universe.cpp \
    evolution.cpp \
    ann_activation_functions.cpp \
    parallel_for_each.cpp \
    thread_pool.cpp \
    ann_dynamic.cpp \
    utils.cpp \
    tournament.cpp

HEADERS += \
    ann_utils.h \
    darwin.h \
    ann_dynamic.h \
    pubsub.h \
    logging.h \
    scope_guard.h \
    matrix.h \
    math_2d.h \
    exception.h \
    properties.h \
    io_utils.h \
    stringify.h \
    platform_abstraction_layer.h \
    database.h \
    format.h \
    universe.h \
    evolution.h \
    ann_activation_functions.h \
    parallel_for_each.h \
    thread_pool.h \
    utils.h \
    pp_utils.h \
    modules.h \
    tournament.h
    
addLibrary(../third_party/sqlite)
addLibrary(../third_party/box2d)
