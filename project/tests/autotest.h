#ifndef AUTOTEST_H
#define AUTOTEST_H
/*
 *
 * From http://qtcreator.blogspot.de/2009/10/running-multiple-unit-tests.html
 *  Thanks!
 *
 */

#include <QTest>
#include <QList>
#include <QString>
#include <QSharedPointer>

#include "gmock/gmock.h"

using ::testing::AtLeast;

namespace AutoTest
{
typedef QList<QObject*> TestList;

inline TestList& testList()
{
    static TestList list;
    return list;
}

inline bool findObject(QObject* object)
{
    TestList& list = testList();
    if (list.contains(object))
    {
        return true;
    }
    foreach (QObject* test, list)
    {
        if (test->objectName() == object->objectName())
        {
            return true;
        }
    }
    return false;
}

inline void addTest(QObject* object)
{
    TestList& list = testList();
    if (!findObject(object))
    {
        list.append(object);
    }
}

inline int run(int argc, char *argv[])
{
    int ret = 0;

    foreach (QObject* test, testList())
    {
        ret += QTest::qExec(test, argc, argv);
    }

    return ret;
}
}

template <class T>
class Test
{
public:
    QSharedPointer<T> child;

    Test(const QString& name) : child(new T)
    {
        child->setObjectName(name);
        AutoTest::addTest(child.data());
    }
};

#define DECLARE_TEST(className) static Test<className> t(#className);

#define TEST_MAIN \
    int main(int argc, char *argv[]) \
{ \
    ::testing::GTEST_FLAG(throw_on_failure) = true;\
    ::testing::InitGoogleMock(&argc, argv);\
    return AutoTest::run(argc, argv); \
    }

#endif // AUTOTEST_H


#if 0
A typical test class header would look something like this:

        #ifndef FOOTESTS_H
        #define FOOTESTS_H

        #include "AutoTest.h"

        class FooTests : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void test1();
    void test2();
    void cleanupTestCase();
};

DECLARE_TEST(FooTests)
#endif // FOOTESTS_H

#endif
