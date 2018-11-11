
include(../../common.pri)

QT += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = conquest_ui
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    new_sandbox_dialog.cpp \
    conquest_ui.cpp \
    board_widget.cpp \
    game_stats_window.cpp \
    game_window.cpp

HEADERS += \
    new_sandbox_dialog.h \
    conquest_ui.h \
    board_widget.h \
    game_stats_window.h \
    game_window.h

FORMS += \
    new_sandbox_dialog.ui \
    game_stats_window.ui \
    game_window.ui

unix {
    target.path = /usr/lib
    INSTALLS += target
}

#------------------------------------------------------------------------------
# core
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../core/release/ -lcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../core/debug/ -lcore
else:unix: LIBS += -L$$OUT_PWD/../../core/ -lcore

DEPENDPATH += $$PWD/../../core

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core/release/libcore.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core/debug/libcore.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core/release/core.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core/debug/core.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../core/libcore.a

#------------------------------------------------------------------------------
# core_ui
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../core_ui/release/ -lcore_ui
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../core_ui/debug/ -lcore_ui
else:unix: LIBS += -L$$OUT_PWD/../../core_ui/ -lcore_ui

DEPENDPATH += $$PWD/../../core_ui

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core_ui/release/libcore_ui.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core_ui/debug/libcore_ui.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core_ui/release/core_ui.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core_ui/debug/core_ui.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../core_ui/libcore_ui.a

#------------------------------------------------------------------------------
# conquest
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../conquest/release/ -lconquest
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../conquest/debug/ -lconquest
else:unix: LIBS += -L$$OUT_PWD/../conquest/ -lconquest

DEPENDPATH += $$PWD/../conquest

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../conquest/release/libconquest.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../conquest/debug/libconquest.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../conquest/release/conquest.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../conquest/debug/conquest.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../conquest/libconquest.a
