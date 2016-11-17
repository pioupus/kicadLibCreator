include(../defaults.pri)

CONFIG( debug, debug|release ) {
    # debug
     TARGET = qTestProjectTemplateAppd
} else {
    # release
    TARGET = qTestProjectTemplateApp

}

TEMPLATE = lib

DEFINES += EXPORT_LIBRARY

SOURCES += main.cpp \
    libcreatorsettings.cpp
SOURCES +=     restrequest.cpp

SOURCES += kicadfile_lib.cpp
SOURCES += octopartinterface.cpp
SOURCES += mainwindow.cpp



HEADERS +=   restrequest.h \
    libcreatorsettings.h

HEADERS += mainwindow.h
HEADERS += kicadfile_lib.h
HEADERS += octopartinterface.h


FORMS    += mainwindow.ui

