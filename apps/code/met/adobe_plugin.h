
#ifndef MET_ADOBE_PLUGIN_H_
#define MET_ADOBE_PLUGIN_H_

#include <vector>
#include <QtCore/QtCore>
#include "onyx/cms/content_manager.h"
#include "plugin_interface.h"

namespace met
{

class AdobePlugin : public PluginInterface
{
public:
    AdobePlugin();
    ~AdobePlugin();

public:
    virtual bool accept(const QFileInfo & info);
    virtual bool extract(const QString & path);
};

};
#endif

