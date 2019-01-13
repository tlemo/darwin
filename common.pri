
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

# Box2D
INCLUDEPATH *= $$PWD/third_party/box2d/src

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

# helper function to generate the library dependency boilerplate
#
# NOTE: it assumes that the library target name is the same as the subdirectory name
#
defineTest(addLibrary) {
    LIB_PATH = $$1
    LIB_NAME = $$basename(LIB_PATH)

    win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/$$LIB_PATH/release/ -l$${LIB_NAME}
    else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/$$LIB_PATH/debug/ -l$${LIB_NAME}
    else:unix: LIBS += -L$$OUT_PWD/$$LIB_PATH/ -l$${LIB_NAME}
    export(LIBS)
    
    DEPENDPATH += $$PWD/$$LIB_PATH
    export(DEPENDPATH)
    
    win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/$$LIB_PATH/release/lib$${LIB_NAME}.a
    else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/$$LIB_PATH/debug/lib$${LIB_NAME}.a
    else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/$$LIB_PATH/release/$${LIB_NAME}.lib
    else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/$$LIB_PATH/debug/$${LIB_NAME}.lib
    else:unix: PRE_TARGETDEPS += $$OUT_PWD/$$LIB_PATH/lib$${LIB_NAME}.a
    export(PRE_TARGETDEPS)
}
