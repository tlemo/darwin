
QT -= core gui

TARGET = sqlite
TEMPLATE = lib
CONFIG += staticlib warn_off

DEFINES += SQLITE_ENABLE_JSON1

SOURCES += \
    sqlite3.c

HEADERS += \
    sqlite3.h
