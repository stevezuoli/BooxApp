#include "comic_model.h"

namespace comic_reader
{

ComicModel::ComicModel()
    : BaseModel()
{
}

ComicModel::~ComicModel(void)
{
}

bool ComicModel::isTheDocument(const QString & path)
{
    return (path_ == path);
}

bool ComicModel::metadata(const MetadataTag tag, QVariant &value)
{
    return false;
}

Configuration & ComicModel::getConf()
{
    return conf_;
}

bool ComicModel::open(const QString & path)
{
    path_ = path;
    return true;
}

bool ComicModel::save()
{
    // TODO
    return false;
}

bool ComicModel::close()
{
    // TODO
    return false;
}

}   // namespace comic_reader
