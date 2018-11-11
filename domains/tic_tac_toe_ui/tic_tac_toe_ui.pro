
QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tic_tac_toe_ui
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    tic_tac_toe_ui.cpp \
    new_sandbox_dialog.cpp \
    sandbox_window.cpp \
    board_widget.cpp \
    game.cpp

HEADERS += \
    tic_tac_toe_ui.h \
    new_sandbox_dialog.h \
    sandbox_window.h \
    board_widget.h \
    human_player.h \
    game.h

FORMS += \
    new_sandbox_dialog.ui \
    sandbox_window.ui

#------------------------------------------------------------------------------
# tic_tac_toe
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../tic_tac_toe/release/ -ltic_tac_toe
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../tic_tac_toe/debug/ -ltic_tac_toe
else:unix: LIBS += -L$$OUT_PWD/../tic_tac_toe/ -ltic_tac_toe

DEPENDPATH += $$PWD/../tic_tac_toe

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../tic_tac_toe/release/libtic_tac_toe.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../tic_tac_toe/debug/libtic_tac_toe.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../tic_tac_toe/release/tic_tac_toe.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../tic_tac_toe/debug/tic_tac_toe.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../tic_tac_toe/libtic_tac_toe.a

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
# common definitions
#
include(../../common.pri)
