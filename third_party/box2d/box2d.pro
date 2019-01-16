
QT -= core gui

TARGET = box2d
TEMPLATE = lib
CONFIG += staticlib warn_off

CONFIG += c++11

CONFIG(debug, debug|release): DEFINES += DEBUG
CONFIG(release, debug|release): DEFINES += NDEBUG

HEADERS += \
    $$files(src/Box2D/*.h, true) \
    box2d.h

SOURCES += \
    $$files(src/Box2D/*.cpp, true)

INCLUDEPATH *= src
