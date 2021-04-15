
QT = gui core network networkauth

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SRC_DIR = $$PWD

QMAKE_CXXFLAGS += -std=c++14
#QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -Werror
QMAKE_CXXFLAGS += -fext-numeric-literals
CONFIG += c++14
#CONFIG += c++11
CONFIG += warn
message($$PWD)

LIBS += -L$$PWD -llibcrypto-1_1-x64 -llibssl-1_1-x64

