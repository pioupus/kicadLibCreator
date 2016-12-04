include(../defaults.pri)

CONFIG( debug, debug|release ) {
    # debug
     TARGET = kikadlibcreatord
} else {
    # release
    TARGET = kikadlibcreator

}

TEMPLATE = lib

DEFINES += EXPORT_LIBRARY

SOURCES += main.cpp \
    libcreatorsettings.cpp \
    ruleeditor.cpp \
    partcreationrule.cpp \
    variablesform.cpp \
    optionsdialog.cpp \
    querymemory.cpp
SOURCES +=     restrequest.cpp

SOURCES += kicadfile_lib.cpp
SOURCES += octopartinterface.cpp
SOURCES += mainwindow.cpp



HEADERS +=   restrequest.h \
    libcreatorsettings.h \
    ruleeditor.h \
    partcreationrule.h \
    variablesform.h \
    optionsdialog.h \
    querymemory.h

HEADERS += mainwindow.h
HEADERS += kicadfile_lib.h
HEADERS += octopartinterface.h


FORMS    += mainwindow.ui \
    ruleeditor.ui \
    variablesform.ui \
    optionsdialog.ui

