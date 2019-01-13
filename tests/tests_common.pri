
include(../common.pri)

TEMPLATE = app
CONFIG += console
CONFIG += thread
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += link_prl

DEFINES += TEST_OUTPUT_PATH=\\\"$$PWD/.output\\\"
DEFINES += TEST_INPUT_PATH=\\\"$$PWD/.input\\\"
DEFINES += TEST_TEMP_PATH=\\\"$$PWD/.temp\\\"

INCLUDEPATH += $$PWD/../third_party/gtest/src/googletest/include
INCLUDEPATH += $$PWD/../third_party/gtest/src/googlemock/include

addLibrary(../../core)
addLibrary(../../registry)
addLibrary(../../third_party/gtest)
