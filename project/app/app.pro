include(../defaults.pri)

TEMPLATE = app
DEFINES += EXPORT_APPLICATION

CONFIG( debug, debug|release ) {
    # debug
     LIBS += -L../src/
} else {
    # release
    LIBS += -L../src/

}


LIBS +=  -lqTestProjectTemplateApp
