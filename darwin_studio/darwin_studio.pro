
QT += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

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

RESOURCES += \
    resources.qrc

#------------------------------------------------------------------------------
# registry
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../registry/release/ -lregistry
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../registry/debug/ -lregistry
else:unix: LIBS += -L$$OUT_PWD/../registry/ -lregistry

DEPENDPATH += $$PWD/../registry

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../registry/release/libregistry.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../registry/debug/libregistry.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../registry/release/registry.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../registry/debug/registry.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../registry/libregistry.a

#------------------------------------------------------------------------------
# core_ui
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../core_ui/release/ -lcore_ui
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../core_ui/debug/ -lcore_ui
else:unix: LIBS += -L$$OUT_PWD/../core_ui/ -lcore_ui

DEPENDPATH += $$PWD/../core_ui

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core_ui/release/libcore_ui.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core_ui/debug/libcore_ui.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core_ui/release/core_ui.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core_ui/debug/core_ui.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../core_ui/libcore_ui.a

#------------------------------------------------------------------------------
# core
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../core/release/ -lcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../core/debug/ -lcore
else:unix: LIBS += -L$$OUT_PWD/../core/ -lcore

DEPENDPATH += $$PWD/../core

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/release/libcore.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/debug/libcore.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/release/core.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/debug/core.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../core/libcore.a

#------------------------------------------------------------------------------
# common
#
include(../common.pri)
