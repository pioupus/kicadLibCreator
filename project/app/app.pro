include(../defaults.pri)

TEMPLATE = app


SOURCES +=  main.cpp

CONFIG(debug, debug|release) {
   # message(Debug)
}

CONFIG( debug, debug|release ) {
    # debug
     #message(Debug)


     LIBS += -L../src/debug/
     LIBS += -L../src/
     LIBS +=  -lkikadlibcreatord

} else {
    # release
    #message(Release)
    LIBS += -L../src/release/
    LIBS += -L../src/
    LIBS +=  -lkikadlibcreator


}

