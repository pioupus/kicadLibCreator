include(../defaults.pri)

TARGET = qTestProjectTemplateApp

TEMPLATE = lib

DEFINES += EXPORT_LIBRARY

SOURCES += main.cpp
SOURCES += mainwindow.cpp

HEADERS += mainwindow.h



FORMS    += mainwindow.ui

#INCLUDEPATH += $$(PYTHON_PATH)/include

#exists( $$(PYTHON_PATH)/include/python.h ) {
#    #message(found python windows)
#    INCLUDEPATH += $$(PYTHON_PATH)/include
#    PYTHON_FOUND = 1
#}

#exists( $$(PYTHON_PATH)/Python.h ) {
    #message(found python linux)crystalTestFrameworkApp
#    INCLUDEPATH += $$(PYTHON_PATH)
#    PYTHON_FOUND = 1
#}

#!equals( PYTHON_FOUND , 1){
#    error (Python directory needs to be configured in environment variable PYTHON_PATH. eg. C:/Python27 )
#}
