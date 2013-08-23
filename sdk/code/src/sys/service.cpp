
#include "onyx/sys/service.h"

namespace sys
{

QString defaultInterface()
{
    QString network = qgetenv("DEFAULT_INTERFACE");
    if (network.isEmpty())
    {
        return "eth0";
    }
    return network;
}

bool checkAndReturnBool(const QList<QVariant> & args)
{
    if (args.size() > 0)
    {
        return args.at(0).toBool();
    }
    return false;
}

}
