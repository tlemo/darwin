
QT -= core gui

TARGET = registry
TEMPLATE = lib
CONFIG += staticlib
CONFIG += create_prl

SOURCES += \
    registry.cpp \
    registry_ui.cpp

HEADERS += \
    registry.h \
    registry_ui.h
    
#------------------------------------------------------------------------------
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../populations/classic/release/ -lclassic
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../populations/classic/debug/ -lclassic
else:unix: LIBS += -L$$OUT_PWD/../populations/classic/ -lclassic

DEPENDPATH += $$PWD/../populations/classic

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../populations/classic/release/libclassic.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../populations/classic/debug/libclassic.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../populations/classic/release/classic.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../populations/classic/debug/classic.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../populations/classic/libclassic.a

#------------------------------------------------------------------------------
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../populations/neat/release/ -lneat
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../populations/neat/debug/ -lneat
else:unix: LIBS += -L$$OUT_PWD/../populations/neat/ -lneat

DEPENDPATH += $$PWD/../populations/neat

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../populations/neat/release/libneat.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../populations/neat/debug/libneat.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../populations/neat/release/neat.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../populations/neat/debug/neat.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../populations/neat/libneat.a

#------------------------------------------------------------------------------
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../domains/pong_ui/release/ -lpong_ui
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../domains/pong_ui/debug/ -lpong_ui
else:unix: LIBS += -L$$OUT_PWD/../domains/pong_ui/ -lpong_ui

DEPENDPATH += $$PWD/../domains/pong_ui

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/pong_ui/release/libpong_ui.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/pong_ui/debug/libpong_ui.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/pong_ui/release/pong_ui.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/pong_ui/debug/pong_ui.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../domains/pong_ui/libpong_ui.a

#------------------------------------------------------------------------------
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../domains/pong/release/ -lpong
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../domains/pong/debug/ -lpong
else:unix: LIBS += -L$$OUT_PWD/../domains/pong/ -lpong

DEPENDPATH += $$PWD/../domains/pong

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/pong/release/libpong.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/pong/debug/libpong.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/pong/release/pong.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/pong/debug/pong.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../domains/pong/libpong.a

#------------------------------------------------------------------------------
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../domains/max/release/ -lmax
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../domains/max/debug/ -lmax
else:unix: LIBS += -L$$OUT_PWD/../domains/max/ -lmax

DEPENDPATH += $$PWD/../domains/max

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/max/release/libmax.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/max/debug/libmax.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/max/release/max.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/max/debug/max.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../domains/max/libmax.a

#------------------------------------------------------------------------------
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../domains/tic_tac_toe_ui/release/ -ltic_tac_toe_ui
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../domains/tic_tac_toe_ui/debug/ -ltic_tac_toe_ui
else:unix: LIBS += -L$$OUT_PWD/../domains/tic_tac_toe_ui/ -ltic_tac_toe_ui

DEPENDPATH += $$PWD/../domains/tic_tac_toe_ui

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/tic_tac_toe_ui/release/libtic_tac_toe_ui.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/tic_tac_toe_ui/debug/libtic_tac_toe_ui.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/tic_tac_toe_ui/release/tic_tac_toe_ui.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/tic_tac_toe_ui/debug/tic_tac_toe_ui.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../domains/tic_tac_toe_ui/libtic_tac_toe_ui.a

#------------------------------------------------------------------------------
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../domains/tic_tac_toe/release/ -ltic_tac_toe
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../domains/tic_tac_toe/debug/ -ltic_tac_toe
else:unix: LIBS += -L$$OUT_PWD/../domains/tic_tac_toe/ -ltic_tac_toe

DEPENDPATH += $$PWD/../domains/tic_tac_toe

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/tic_tac_toe/release/libtic_tac_toe.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/tic_tac_toe/debug/libtic_tac_toe.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/tic_tac_toe/release/tic_tac_toe.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/tic_tac_toe/debug/tic_tac_toe.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../domains/tic_tac_toe/libtic_tac_toe.a

#------------------------------------------------------------------------------
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../domains/conquest_ui/release/ -lconquest_ui
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../domains/conquest_ui/debug/ -lconquest_ui
else:unix: LIBS += -L$$OUT_PWD/../domains/conquest_ui/ -lconquest_ui

DEPENDPATH += $$PWD/../domains/conquest_ui

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/conquest_ui/release/libconquest_ui.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/conquest_ui/debug/libconquest_ui.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/conquest_ui/release/conquest_ui.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/conquest_ui/debug/conquest_ui.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../domains/conquest_ui/libconquest_ui.a

#------------------------------------------------------------------------------
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../domains/conquest/release/ -lconquest
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../domains/conquest/debug/ -lconquest
else:unix: LIBS += -L$$OUT_PWD/../domains/conquest/ -lconquest

DEPENDPATH += $$PWD/../domains/conquest

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/conquest/release/libconquest.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/conquest/debug/libconquest.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/conquest/release/conquest.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/conquest/debug/conquest.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../domains/conquest/libconquest.a

#------------------------------------------------------------------------------
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../domains/harvester_ui/release/ -lharvester_ui
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../domains/harvester_ui/debug/ -lharvester_ui
else:unix: LIBS += -L$$OUT_PWD/../domains/harvester_ui/ -lharvester_ui

DEPENDPATH += $$PWD/../domains/harvester_ui

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/harvester_ui/release/libharvester_ui.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/harvester_ui/debug/libharvester_ui.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/harvester_ui/release/harvester_ui.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/harvester_ui/debug/harvester_ui.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../domains/harvester_ui/libharvester_ui.a

#------------------------------------------------------------------------------
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../domains/harvester/release/ -lharvester
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../domains/harvester/debug/ -lharvester
else:unix: LIBS += -L$$OUT_PWD/../domains/harvester/ -lharvester

DEPENDPATH += $$PWD/../domains/harvester

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/harvester/release/libharvester.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/harvester/debug/libharvester.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/harvester/release/harvester.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../domains/harvester/debug/harvester.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../domains/harvester/libharvester.a

#------------------------------------------------------------------------------
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
# common definitions
#
include(../common.pri)
