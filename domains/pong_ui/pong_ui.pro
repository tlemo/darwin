
include(../../common.pri)

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = pong_ui
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    pong_widget.cpp \
    game_widget.cpp \
    new_sandbox_dialog.cpp \
    pong_ui.cpp

HEADERS += \
    pong_widget.h \
    game_widget.h \
    human_player.h \
    pong_ui.h \
    new_sandbox_dialog.h

FORMS += \
    game_widget.ui \
    new_sandbox_dialog.ui

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
# pong
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../pong/release/ -lpong
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../pong/debug/ -lpong
else:unix: LIBS += -L$$OUT_PWD/../pong/ -lpong

DEPENDPATH += $$PWD/../pong

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../pong/release/libpong.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../pong/debug/libpong.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../pong/release/pong.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../pong/debug/pong.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../pong/libpong.a
