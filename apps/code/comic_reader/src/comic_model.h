#ifndef COMIC_MODEL_H_
#define COMIC_MODEL_H_

#include "onyx/base/base_model.h"

using namespace vbf;

namespace comic_reader
{

class ComicModel : public BaseModel
{
    Q_OBJECT

public:
    ComicModel();
    ~ComicModel(void);

public:
    inline const QString & path() const { return path_; }
    virtual bool isTheDocument(const QString &path);
    virtual bool metadata(const MetadataTag tag, QVariant &value);
    Configuration & getConf();

    bool open(const QString & path);
    bool save();
    bool close();

private:
    Configuration conf_;
    QString path_;
};

}   // namespace comic_reader

#endif
