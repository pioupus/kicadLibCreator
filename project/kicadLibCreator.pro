#-------------------------------------------------
#
# Project created by QtCreator 2015-05-07T17:15:19
#
#-------------------------------------------------


TARGET = kicadlibcreator
TEMPLATE = subdirs

SUBDIRS = src
SUBDIRS += app
#SUBDIRS += tests

app.depends = src
#tests.depends = src

CONFIG += static
CONFIG += c++17

