
QT -= core gui

TARGET = tinyspline
TEMPLATE = lib
CONFIG += staticlib warn_off

CONFIG += c++11

CONFIG(debug, debug|release): DEFINES += DEBUG
CONFIG(release, debug|release): DEFINES += NDEBUG

HEADERS += \
    $$files(repo/src/*.h, true) \
    tinyspline.h

SOURCES += \
    $$files(repo/src/*.c, true) \
    $$files(repo/src/*.cxx, true)
