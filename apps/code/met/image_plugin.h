
#ifndef MET_IMAGE_PLUGIN_H_
#define MET_IMAGE_PLUGIN_H_

#include <vector>
#include <QtCore/QtCore>
#include "onyx/cms/content_manager.h"
#include "plugin_interface.h"

namespace met
{

class ImagePlugin : public PluginInterface
{
public:
    ImagePlugin();
    ~ImagePlugin();

public:
    virtual bool accept(const QFileInfo & info);
    virtual bool extract(const QString & path);
};

};
#endif

