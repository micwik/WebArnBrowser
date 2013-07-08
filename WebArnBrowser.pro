#-------------------------------------------------
#
# Project created by QtCreator 2012-05-20T23:53:09
#
#-------------------------------------------------

QT += core
QT += network
QT += gui

QMAKE_CXXFLAGS += -DWT_NO_SLOT_MACROS

TARGET = WebArnBrowser
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
OBJECTS_DIR = tmp
MOC_DIR = tmp


SOURCES += src/wqlib/DispatchThread.cpp \
    src/wqlib/WQApplication.cpp \
    wt/WCodeEdit.cpp \
    src/main.cpp \
    src/Math.cpp \
    src/MainWindow.cpp \
    src/QtMainThread.cpp \
    src/MultiDelegate.cpp \
    src/Session.cpp \
    src/User.cpp \
    src/TermWindow.cpp \
    src/CodeWindow.cpp \
    src/VcsWindow.cpp \
    src/ManageWindow.cpp \
    src/ArnModel.cpp

HEADERS += \
    src/wqlib/DispatchThread.hpp \
    src/wqlib/WQApplication.hpp \
    wt/DomElement.h \
    wt/EscapeOStream.h \
    wt/WCodeEdit.h \
    src/Math.hpp \
    src/MainWindow.hpp \
    src/QtMainThread.hpp \
    src/MultiDelegate.hpp \
    src/User.hpp \
    src/Session.hpp \
    src/ItemDataRole.hpp \
    src/TermWindow.hpp \
    src/CodeWindow.hpp \
    src/VcsWindow.hpp \
    src/ManageWindow.hpp \
    src/ArnModel.hpp


LIBS += -L$$OUT_PWD/../ArnLib/ -lArn -lwt -lwthttp -lwtdbo -lwtdbosqlite3 \
        -lboost_thread -lboost_signals -lboost_system \
        -lcrypt

INCLUDEPATH += $$PWD/.. /usr/local/include/wt

OTHER_FILES += \
    logout.html \
    styles.css




















