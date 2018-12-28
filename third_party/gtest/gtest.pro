
QT -= core gui

TARGET = gtest
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    src/googletest/src/gtest-all.cc \
    src/googlemock/src/gmock-all.cc

HEADERS += \
    gtest.h \
    gmock.h
    
INCLUDEPATH *= \
    src/googletest \
    src/googletest/include \
    src/googlemock \
    src/googlemock/include
    
unix {
    target.path = /usr/lib
    INSTALLS += target
}
