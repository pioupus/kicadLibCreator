
QT = gui core network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SRC_DIR = $$PWD

QMAKE_CXXFLAGS += -std=c++14
QMAKE_CXXFLAGS += -Werror
CONFIG += c++14
CONFIG += warn




