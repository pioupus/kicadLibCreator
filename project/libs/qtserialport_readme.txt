From: https://wiki.qt.io/QtSerialPort

to download submodule use

git submodule init
git submodule update

then checkout branch according your qt version eg:

git checkout v5.2.1

cd libs/build/qtserialport
qmake ../../qtserialport/qtserialport.pro
make