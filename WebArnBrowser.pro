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


SOURCES += main.cpp \
    wqlib/DispatchThread.cpp \
    wqlib/WQApplication.cpp \
    Math.cpp \
    MainWindow.cpp \
    QtMainThread.cpp \
    MultiDelegate.cpp \
    Session.cpp \
    User.cpp \
    TermWindow.cpp \
    CodeWindow.cpp \
    wt/WCodeEdit.cpp \
    VcsWindow.cpp \
    ManageWindow.cpp \
    ArnModel.cpp

HEADERS += \
    wqlib/DispatchThread.hpp \
    wqlib/WQApplication.hpp \
    Math.hpp \
    MainWindow.hpp \
    QtMainThread.hpp \
    MultiDelegate.hpp \
    User.hpp \
    Session.hpp \
    ItemDataRole.hpp \
    TermWindow.hpp \
    CodeWindow.hpp \
    wt/DomElement.h \
    wt/EscapeOStream.h \
    wt/WCodeEdit.h \
    VcsWindow.hpp \
    ManageWindow.hpp \
    ArnModel.hpp


LIBS += -L$$OUT_PWD/../ArnLib/ -lArn -lwt -lwthttp -lwtdbo -lwtdbosqlite3 \
        -lboost_thread -lboost_signals -lboost_system \
        -lcrypt
#LIBS += -L$$OUT_PWD/../ArnLib/ -lArn -L/usr/local/lib/ -lwt -lwthttp -lboost_thread -lboost_signals

#INCLUDEPATH += $$PWD/..
INCLUDEPATH += $$PWD/.. /usr/local/include/wt
#INCLUDEPATH = $$PWD/.. /usr/local/include/wt $$INCLUDEPATH

OTHER_FILES += \
    logout.html \
    styles.css




















