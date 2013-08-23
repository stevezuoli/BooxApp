#include "cache_files.h"

namespace compression
{

/*
 * Cache maximum to 10 files.
 */
// TODO jim: for enhancement, can use the total size of all files as the limit.
const unsigned int CacheFiles::LOAD_FILE_LIMIT = 10;

CacheFiles::CacheFiles()
{

}

CacheFiles::~CacheFiles()
{
}

int CacheFiles::add(shared_ptr<ArchiveFileItem> ptr)
{
    int index = files_.indexOf(ptr);
    if (index < 0)
    {
        if (size() >= LOAD_FILE_LIMIT)
        {

        }
        else
        {
            files_.push_back(ptr);
        }
    }
    else
    {
        files_[index] = ptr;
    }
    return index;
}

shared_ptr<ArchiveFileItem> CacheFiles::getFile(int index)
{
    return files_[index];
}

void CacheFiles::clear()
{
    files_.clear();
}

size_t CacheFiles::size()
{
    return files_.size();
}

}   // namespace compression
