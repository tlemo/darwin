
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
    roulette_selection.cpp \
    cgp_islands_selection.cpp \
    truncation_selection.cpp \
    simple_tournament.cpp \
    swiss_tournament.cpp \
    sim/accelerometer.cpp \
    sim/camera.cpp \
    sim/compass.cpp \
    sim/scene.cpp \
    sim/misc.cpp \
    sim/touch_sensor.cpp \
    sim/drone.cpp \
    sim/drone_controller.cpp

HEADERS += \
    ann_utils.h \
    darwin.h \
    ann_dynamic.h \
    global_initializer.h \
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
    tournament.h \
    selection_algorithm.h \
    roulette_selection.h \
    cgp_islands_selection.h \
    truncation_selection.h \
    simple_tournament.h \
    tournament_implementations.h \
    swiss_tournament.h \
    sim/accelerometer.h \
    sim/camera.h \
    sim/compass.h \
    sim/scene.h \
    sim/misc.h \
    sim/script.h \
    sim/touch_sensor.h \
    sim/drone.h \
    sim/drone_controller.h
    
addLibrary(../third_party/sqlite)
addLibrary(../third_party/box2d)
addLibrary(../third_party/tinyspline)
addSystemLibraries()
