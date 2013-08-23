
#include "adobe_plugin.h"

namespace met
{

AdobePlugin::AdobePlugin()
{
}

AdobePlugin::~AdobePlugin()
{
}

bool AdobePlugin::accept(const QFileInfo & info)
{
    if (info.suffix().compare("pdf", Qt::CaseInsensitive) == 0 ||
        info.suffix().compare("epub", Qt::CaseInsensitive) == 0)
    {
        return true;
    }
    return false;
}

bool AdobePlugin::extract(const QString & path)
{
    QStringList args;
    args << path;
    args << "metadata";

    QProcess process;
    process.start("naboo_reader", args);
    if (!process.waitForStarted())
    {
        return false;
    }

    process.waitForFinished(-1);
    return true;
}

}

