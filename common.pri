
# compiler switches
win32-msvc {
    QMAKE_CXXFLAGS += /permissive- /std:c++17
    QMAKE_CXXFLAGS += /wd4244 # conversion from 'double' to 'float', possible loss of data
    QMAKE_CXXFLAGS += /wd5030 # attribute is not recognized
} else {
    QMAKE_CXXFLAGS += -mavx2 -mfma
    QMAKE_CXXFLAGS += -Wno-reorder

    # TODO: re-enable once the size_t mess is cleaned up
    QMAKE_CXXFLAGS += -Wno-sign-compare
    QMAKE_CXXFLAGS += -Wno-type-limits

    QMAKE_CXXFLAGS += -Wno-deprecated-declarations
}

CONFIG += c++1z

CONFIG(release, debug|release): DEFINES += NDEBUG

INCLUDEPATH += $$PWD

# libraries
unix {
    clang: LIBS += -ldl -lc++fs
    else: LIBS += -ldl -lstdc++fs
}

# workarounds for mingw/glibc
win32-g++ {
    DEFINES += __STDC_FORMAT_MACROS
    LIBS += -lstdc++fs
}

