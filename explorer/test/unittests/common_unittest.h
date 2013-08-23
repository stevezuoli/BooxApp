// Copyright 2007 Company Name. All Rights Reserved.
// Author: John

#ifndef COMMON_UNITTEST_H_
#define COMMON_UNITTEST_H_

#include <boost/filesystem.hpp>
#include "onyx/base/base.h"

namespace common_unittest
{

struct FileInfo
{
    base::string path;
    unsigned int size;
};

void CreateTempFile(const base::string & path_name,
                    const unsigned int size);

void RemoveFile(const base::string & path_name);

void CreateTempDirectory(const boost::filesystem::path & path_name,
                         const FileInfo * tmp_files,
                         const int size);

void RemoveDirectory(const base::string &path);


}

#endif
