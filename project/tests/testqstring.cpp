#include "testqstring.h"




void TestQString::initTestCase(){

}

void TestQString::cleanupTestCase(){

}

void TestQString::toUpper()
{
    QString str = "Hello";
    QCOMPARE(str.toUpper(), QString("HELLO"));
}
