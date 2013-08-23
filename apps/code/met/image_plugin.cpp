
#include "image_plugin.h"
#include "onyx/cms/content_thumbnail.h"

namespace met
{

ImagePlugin::ImagePlugin()
{
}

ImagePlugin::~ImagePlugin()
{
}

bool ImagePlugin::accept(const QFileInfo & info)
{
    return cms::isImage(info.suffix());
}

bool ImagePlugin::extract(const QString & path)
{
    return cms::makeThumbnail(path);
}

}

