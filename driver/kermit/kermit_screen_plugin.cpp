#include <qscreendriverplugin_qws.h>
#include <qstringlist.h>
#include "kermit_screen.h"

QT_BEGIN_NAMESPACE

class KermitScreenPlugin : public QScreenDriverPlugin
{
public:
    KermitScreenPlugin();

    QStringList keys() const;
    QScreen *create(const QString&, int displayId);
};

KermitScreenPlugin::KermitScreenPlugin()
    : QScreenDriverPlugin()
{
}

QStringList KermitScreenPlugin::keys() const
{
    QStringList list;
    list << QLatin1String("KermitScreen");
    return list;
}

QScreen* KermitScreenPlugin::create(const QString& driver, int displayId)
{
    if (driver.toLower() == QLatin1String("kermitscreen"))
        return new KermitScreen(displayId);

    return 0;
}

Q_EXPORT_PLUGIN2(KermitScreen, KermitScreenPlugin)

QT_END_NAMESPACE


