#include(libs/PythonQt3.0/build/python.prf)

QT = gui core network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SRC_DIR = $$PWD

INCLUDEPATH += $$PWD/src
INCLUDEPATH += $$PWD/src/CommunicationDevices

INCLUDEPATH += $$PWD/libs/googletest/googletest/include
INCLUDEPATH += $$PWD/libs/googletest/googlemock/include

LIBS += -L$$PWD/libs/googletest/build/googlemock/
LIBS += -L$$PWD/libs/googletest/build/googlemock/gtest


#exists( $$PWD/libs/PythonQt3.0/src/PythonQt.h ) {
    #message(found python windows)
#    INCLUDEPATH += $$PWD/libs/PythonQt3.0/src/
#    LIBS += -L$$PWD/libs/PythonQt3.0/lib
#   PYTHONQT_FOUND = 1
#}

#exists( $$(PYTHONQT_PATH)/src/PythonQt.h ) {
    #message(found python windows)
#    INCLUDEPATH += $$(PYTHONQT_PATH)/src/PythonQt.h
#    LIBS += -L$$(PYTHONQT_PATH)/lib
#    PYTHONQT_FOUND = 1
#}

#exists( $$(PYTHONQT_PATH)/PythonQt.h ) {
    #message(found PythonQt linux)
#    INCLUDEPATH += $$(PYTHONQT_PATH)
#    PYTHONQT_FOUND = 1
#}

#!equals( PYTHONQT_FOUND , 1){
#    error (PythonQT directory needs to be configured in environment variable PYTHONQT_PATH. )
#}



#LIBS += -lPythonQt  #-lPythonQt_QtAll

CONFIG += c++14
CONFIG += warn
QMAKE_CXXFLAGS += -Werror
