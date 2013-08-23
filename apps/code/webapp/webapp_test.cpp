#include "webapp_test.h"

namespace webapp
{

TestObject::TestObject(QObject *parent)
: QObject(parent)
{
}

TestObject::~TestObject()
{
}

void TestObject::testMap(const QVariantMap & map)
{
    qDebug("testMap");
    QVariantMap::const_iterator iter = map.begin();
    for (; iter != map.end(); ++iter)
    {
        int value = iter.value().toInt();
        qDebug("Map value:%d", value);
    }
}

void TestObject::testVec(const QVariantList & list)
{
    qDebug("testVector");
    QVariantList::const_iterator iter = list.begin();
    for (; iter != list.end(); ++iter)
    {
        int value = (*iter).toInt();
        qDebug("List value:%d", value);
    }
}

QVariantList TestObject::testGetList()
{
    QVariantList list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);
    return list;
}

}