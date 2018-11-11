
include(../../common.pri)

QT -= core gui

TARGET = pong
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    game.cpp \
    pong.cpp \
    ann_player.cpp \
    test_players.cpp

HEADERS += \
    player.h \
    game.h \
    pong.h \
    ann_player.h \
    test_players.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../core/release/ -lcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../core/debug/ -lcore
else:unix: LIBS += -L$$OUT_PWD/../../core/ -lcore

DEPENDPATH += $$PWD/../../core

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core/release/libcore.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core/debug/libcore.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core/release/core.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core/debug/core.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../core/libcore.a
