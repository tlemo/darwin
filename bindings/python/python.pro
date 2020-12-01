
include(../../common.pri)

TARGET=darwin
TEMPLATE = lib
CONFIG -= qt
CONFIG += link_prl

# disable the 'lib' shared library prefix and version symlinks
CONFIG += plugin no_plugin_name_prefix

# avoid the MSVC-style extra debug/release subdirectories in Windows builds
CONFIG -= debug_and_release

win32: QMAKE_EXTENSION_SHLIB = pyd

SOURCES += \
    python_bindings.cpp

HEADERS += \
    python_bindings.h

# include path for Python headers
INCLUDEPATH += $$(PYTHON_INCLUDE)

# Python library path (should only be needed on Windows)
PYTHON_LIBS = $$(PYTHON_LIBS)
!isEmpty(PYTHON_LIBS) {
    LIBS += "-L$$PYTHON_LIBS"
}

addLibrary(../../registry)
addLibrary(../../core)

# default rules for deployment
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
