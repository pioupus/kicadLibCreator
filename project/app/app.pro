include(../defaults.pri)

TEMPLATE = app
TARGET = kicadlibcreator

SOURCES +=  main.cpp

CONFIG(debug, debug|release) {
   # message(Debug)
}

CONFIG( debug, debug|release ) {
    # debug
     #message(Debug)


     LIBS += -L../src/debug/
     LIBS += -L../src/
     LIBS +=  -lkicadlibcreatord

} else {
    # release
    #message(Release)
    LIBS += -L../src/release/
    LIBS += -L../src/
    LIBS +=  -lkicadlibcreator


}

