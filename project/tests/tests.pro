include(../defaults.pri)

TEMPLATE = app
CONFIG += console
QT +=  testlib

DEFINES += EXPORT_APPLICATION

HEADERS += autotest.h \
    testqstring.h

SOURCES +=  main.cpp
SOURCES +=  testqstring.cpp

INCLUDEPATH += $$PWD/libs/googletest/googletest/include
INCLUDEPATH += $$PWD/libs/googletest/googlemock/include

LIBS += -L$$PWD/libs/build/googletest/googlemock/
LIBS += -L$$PWD/libs/build/googletest/googlemock/gtest


LIBS += -lgmock
LIBS += -lgtest


CONFIG( debug, debug|release ) {
    # debug
    LIBS += -L../src
} else {
    # release
    LIBS += -L../src/
}

COPY_DIR = "$$(UNIXTOOLS)cp -r"

#message($$COPY_DIR)
#message($$OUT_PWD/)
#message($$PWD/scripts)

#copies scripts into builds

#runtests.commands = $$RUNTEST
#runtests.depends = copydata

copydata.commands = $$COPY_DIR $$PWD/scripts $$OUT_PWD/
first.depends = $(first) copydata

export(first.depends)
export(copydata.commands)

QMAKE_EXTRA_TARGETS +=   first copydata

LIBS +=  -lqTestProjectTemplateApp


#
