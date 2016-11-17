#ifndef TESTQSTRING_H
#define TESTQSTRING_H

#include <QObject>
#include "autotest.h"

class FooClass {
  virtual void PenUp() = 0;
  virtual void PenDown() = 0;
};

class TestKICADLibSchematicDeviceLibrary : public QObject
{
    Q_OBJECT

private slots:
	void initTestCase();
	void cleanupTestCase();
    void test_loadfromFile();
    void test_loadfromFile_brooktre();

    void test_loadDef();
    void test_loadFields();
    void test_loadDrawElements();

    void test_saveDeviceIntoLibrary();

    void test_BasicMocking();
    void test_splitParams();
    void test_description();
    void test_indexOf();
    void test_insertDevice();
};
DECLARE_TEST(TestKICADLibSchematicDeviceLibrary)
#endif // TESTQSTRING_H
