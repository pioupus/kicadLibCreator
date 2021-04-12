#ifndef TESTQSTRING_H
#define TESTQSTRING_H

#include "autotest.h"
#include <QObject>

class TestQString : public QObject {
    Q_OBJECT

    private slots:
    void initTestCase();
    void cleanupTestCase();
    void toUpper();
};
//DECLARE_TEST(TestQString)
#endif // TESTQSTRING_H
