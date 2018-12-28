
TEMPLATE = app
CONFIG += console
CONFIG += thread
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += link_prl

DEFINES += TEST_OUTPUT_PATH=\\\"$$PWD/.output\\\"
DEFINES += TEST_INPUT_PATH=\\\"$$PWD/.input\\\"
DEFINES += TEST_TEMP_PATH=\\\"$$PWD/.temp\\\"

#------------------------------------------------------------------------------
# registry
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../registry/release/ -lregistry
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../registry/debug/ -lregistry
else:unix: LIBS += -L$$OUT_PWD/../../registry/ -lregistry

DEPENDPATH += $$PWD/../../registry

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../registry/release/libregistry.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../registry/debug/libregistry.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../registry/release/registry.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../registry/debug/registry.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../registry/libregistry.a

#------------------------------------------------------------------------------
# core
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../core/release/ -lcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../core/debug/ -lcore
else:unix: LIBS += -L$$OUT_PWD/../../core/ -lcore

DEPENDPATH += $$PWD/../core

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core/release/libcore.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core/debug/libcore.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core/release/core.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core/debug/core.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../core/libcore.a

#------------------------------------------------------------------------------
# gtest
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../third_party/gtest/release/ -lgtest
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../third_party/gtest/debug/ -lgtest
else:unix: LIBS += -L$$OUT_PWD/../../third_party/gtest/ -lgtest

# TODO: I don't like this hack
INCLUDEPATH += $$PWD/../third_party/gtest/src/googletest/include
INCLUDEPATH += $$PWD/../third_party/gtest/src/googlemock/include

DEPENDPATH += $$PWD/../third_party/gtest

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../third_party/gtest/release/libgtest.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../third_party/gtest/debug/libgtest.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../third_party/gtest/release/gtest.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../third_party/gtest/debug/gtest.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../third_party/gtest/libgtest.a

#------------------------------------------------------------------------------
# common definitions
#
include(../common.pri)
