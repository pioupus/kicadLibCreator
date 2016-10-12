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

    void test_BasicMocking();
};
DECLARE_TEST(TestKICADLibSchematicDeviceLibrary)
#endif // TESTQSTRING_H
