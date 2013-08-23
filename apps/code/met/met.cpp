
#include "met.h"
#include "image_plugin.h"
#include "adobe_plugin.h"


namespace met
{

Manager::Manager()
{
    loadPlugins();
}

Manager::~Manager()
{
}

bool Manager::extract(const QString & path)
{
    QFileInfo info(path);
    foreach(PluginInterfacePtr p, plugins_)
    {
        if (p->accept(info))
        {
            return p->extract(path);
        }
    }
    return false;
}

void Manager::loadPlugins()
{
    if (plugins_.size() <= 0)
    {
        plugins_.push_back(new ImagePlugin);
        plugins_.push_back(new AdobePlugin);
    }
}


};

