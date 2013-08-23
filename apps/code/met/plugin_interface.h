
#ifndef MET_PLUGIN_INTERFACE_H_
#define MET_PLUGIN_INTERFACE_H_

#include <QVector>
#include <QtCore/QtCore>
#include "onyx/cms/content_manager.h"

namespace met
{

class PluginInterface
{
public:
    PluginInterface() {}
    ~PluginInterface() {}

public:
    virtual bool accept(const QFileInfo & info) = 0;
    virtual bool extract(const QString & path) = 0;
};
typedef PluginInterface * PluginInterfacePtr;
typedef QVector<PluginInterface *> Plugins;

};
#endif

