include(../defaults.pri)

CONFIG( debug, debug|release ) {
    # debug
     TARGET = kicadlibcreatord
} else {
    # release
    TARGET = kicadlibcreator

}

TEMPLATE = lib

DEFINES += EXPORT_LIBRARY

SOURCES += main.cpp \
    libcreatorsettings.cpp \
    remotedatasource.cpp \
    ruleeditor.cpp \
    partcreationrule.cpp \
    variablesform.cpp \
    optionsdialog.cpp \
    querymemory.cpp \
    textfile.cpp \
    designsettings.cpp \
    renderarea.cpp \
    digikeywrapper.cpp

SOURCES +=     restrequest.cpp

SOURCES += kicadfile_lib.cpp
SOURCES += octopartinterface.cpp
SOURCES += mainwindow.cpp



HEADERS +=   restrequest.h \
    libcreatorsettings.h \
    remotedatasource.h \
    ruleeditor.h \
    partcreationrule.h \
    variablesform.h \
    optionsdialog.h \
    querymemory.h \
    textfile.h \
    designsettings.h \
    renderarea.h \
    digikeywrapper.h

HEADERS += mainwindow.h
HEADERS += kicadfile_lib.h
HEADERS += octopartinterface.h


FORMS    += mainwindow.ui \
    ruleeditor.ui \
    variablesform.ui \
    optionsdialog.ui \
    textfile.ui \
    designsettings.ui

