#ifndef WEB_TEST_H_
#define WEB_TEST_H_

#include "onyx/ui/ui.h"

using namespace ui;

namespace webapp
{

class TestObject : public QObject
{
    Q_OBJECT
public:
    TestObject(QObject *parent = 0);
    ~TestObject();

public Q_SLOTS:
    void testMap(const QVariantMap & map);
    void testVec(const QVariantList & list);
    QVariantList testGetList();
};

};   // namespace webapp

#endif
