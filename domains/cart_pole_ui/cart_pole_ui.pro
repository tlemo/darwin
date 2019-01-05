
include(../../common.pri)

QT += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cart_pole_ui
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    cart_pole_ui.cpp \
    new_sandbox_dialog.cpp \
    world_widget.cpp \
    sandbox_window.cpp \
    box2d_renderer.cpp

HEADERS += \
    cart_pole_ui.h \
    new_sandbox_dialog.h \
    world_widget.h \
    sandbox_window.h \
    box2d_renderer.h

FORMS += \
    new_sandbox_dialog.ui \
    sandbox_window.ui

unix {
    target.path = /usr/lib
    INSTALLS += target
}

#------------------------------------------------------------------------------
# core
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../core/release/ -lcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../core/debug/ -lcore
else:unix: LIBS += -L$$OUT_PWD/../../core/ -lcore

DEPENDPATH += $$PWD/../../core

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core/release/libcore.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core/debug/libcore.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core/release/core.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core/debug/core.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../core/libcore.a

#------------------------------------------------------------------------------
# core_ui
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../core_ui/release/ -lcore_ui
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../core_ui/debug/ -lcore_ui
else:unix: LIBS += -L$$OUT_PWD/../../core_ui/ -lcore_ui

DEPENDPATH += $$PWD/../../core_ui

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core_ui/release/libcore_ui.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core_ui/debug/libcore_ui.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core_ui/release/core_ui.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../core_ui/debug/core_ui.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../core_ui/libcore_ui.a

#------------------------------------------------------------------------------
# cart_pole
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../cart_pole/release/ -lcart_pole
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../cart_pole/debug/ -lcart_pole
else:unix: LIBS += -L$$OUT_PWD/../cart_pole/ -lcart_pole

DEPENDPATH += $$PWD/../cart_pole

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../cart_pole/release/libcart_pole.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../cart_pole/debug/libcart_pole.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../cart_pole/release/cart_pole.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../cart_pole/debug/cart_pole.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../cart_pole/libcart_pole.a
