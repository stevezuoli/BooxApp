#ifndef ONYX_SERVICE_H_
#define ONYX_SERVICE_H_

#include <QtCore/QtCore>

namespace sys
{

static const QString service = "com.onyx.service.system_manager";
static const QString object  = "/com/onyx/object/system_manager";
static const QString iface   = "com.onyx.interface.system_manager";
QString defaultInterface();

bool checkAndReturnBool(const QList<QVariant> & args);

}  // namespace sys


#endif
