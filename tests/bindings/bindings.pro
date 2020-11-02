
include(../tests_common.pri)

SOURCES += \
    main.cpp \
    python_tests.cpp

DISTFILES += \
    $$files(python/*.py, true)

