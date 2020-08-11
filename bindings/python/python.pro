
include(../../common.pri)

TARGET=darwin
TEMPLATE = lib
CONFIG -= qt
CONFIG += link_prl

# disable the 'lib' shared library prefix and version symlinks
CONFIG += plugin no_plugin_name_prefix

SOURCES += \
    python_bindings.cpp

HEADERS += \
    python_bindings.h

# include path for Python headers
INCLUDEPATH += $$(PYTHON_INCLUDE)

addLibrary(../../registry)
addLibrary(../../core)

# default rules for deployment
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
