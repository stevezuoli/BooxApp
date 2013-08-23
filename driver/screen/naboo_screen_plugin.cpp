#include <qscreendriverplugin_qws.h>
#include <qstringlist.h>
#include "naboo_screen.h"

QT_BEGIN_NAMESPACE

class NabooScreenPlugin : public QScreenDriverPlugin
{
public:
    NabooScreenPlugin();

    QStringList keys() const;
    QScreen *create(const QString&, int displayId);
};

NabooScreenPlugin::NabooScreenPlugin()
    : QScreenDriverPlugin()
{
}

QStringList NabooScreenPlugin::keys() const
{
    QStringList list;
    list << QLatin1String("OnyxScreen");
    list << QLatin1String("NabooScreen");
    return list;
}

QScreen* NabooScreenPlugin::create(const QString& driver, int displayId)
{
    if (driver.compare("onyxscreen", Qt::CaseInsensitive) == 0 ||
        driver.compare("nabooscreen", Qt::CaseInsensitive) == 0)
    {
        return new NabooScreen(displayId);
    }

    return 0;
}

Q_EXPORT_PLUGIN2(NabooScreen, NabooScreenPlugin)

QT_END_NAMESPACE


