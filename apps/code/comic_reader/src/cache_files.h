#ifndef CACHE_FILES_H_
#define CACHE_FILES_H_

#include "onyx/base/base.h"
#include "archive_file_item.h"

namespace compression
{

class CacheFiles
{
public:
    CacheFiles();
    ~CacheFiles();

    int add(shared_ptr<ArchiveFileItem> p);
    void clear();
    size_t size();
    shared_ptr<ArchiveFileItem> getFile(int index);

private:
    static const unsigned int LOAD_FILE_LIMIT;

    typedef QVector<shared_ptr<ArchiveFileItem> > Files;
    typedef Files::iterator FilesIter;
    Files files_;
};

}   // namespace compression

#endif
