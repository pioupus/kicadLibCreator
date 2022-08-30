#!/bin/sh
#BUILD_FOLDER=builds/Release
BUILD_FOLDER=build-kicadLibCreator-Desktop_Qt_5_15_2_GCC_64bit-Debug/
#BUILD_FOLDER=build-kicadLibCreator-Desktop-Debug/
export LD_LIBRARY_PATH=$BUILD_FOLDER/src
$BUILD_FOLDER/app/kicadlibcreator
