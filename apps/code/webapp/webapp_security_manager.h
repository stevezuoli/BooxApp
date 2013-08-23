#ifndef WEB_SECURITY_MANAGER_H_
#define WEB_SECURITY_MANAGER_H_

#include "onyx/base/base.h"
#include "onyx/base/down_cast.h"
#include "onyx/ui/ui.h"

using namespace ui;

namespace webapp
{

class SecurityManager : public QObject
{
    Q_OBJECT
public:
    SecurityManager(QObject *parent = 0);
    ~SecurityManager();

public Q_SLOTS:
    QVariantList generateCredentials();
    QString getClientId();
    QString getAuthKey();

private:

};

};   // namespace webapp

#endif
